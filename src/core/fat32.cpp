#include "core/fat32.hpp"
#include "core/module.hpp"
#include "core/vga.hpp"
#include "core/mm.hpp"
#include "core/string.hpp"
#include "core/list.hpp"
#include "core/ascii.hpp"

constexpr uint32_t bpbSectorSizeOffset = 0xB;
constexpr uint32_t bpbClusterSizeOffset = 0xD;
constexpr uint32_t bpbReservedSectorsOffset = 0xE;
constexpr uint32_t bpbNumberOfFats = 0x10;
constexpr uint32_t bpbSectorsPerFatOffset = 0x24;
constexpr uint32_t bpbRootDirClusterOffset = 0x2C;
constexpr uint32_t bpbFsInfoStrOffset = 0x52;
constexpr size_t bpbFsInfoStrSize = 8;

constexpr size_t sectorSize = 512;

constexpr uint32_t invalidClusterNum = 0xFFFFFFFF;

FS::Context* ctx;

struct PartitionInfo
{
	uint32_t lba;
	uint32_t fatLba;
	uint32_t rootDirLba;
	uint32_t sectorSize; // bytes per sector
	uint32_t clusterSize; // sectors per cluster
	uint32_t sectorsPerFat; // number of sectors per fat
	uint32_t numberOfFats;
	uint32_t dataLba;
};

struct DirEntry
{
	char name[11];
	char attributes;
	char unused[8];
	uint16_t highClusterNum;
	uint16_t modifiedTime;
	uint16_t modifiedDate;
	uint16_t lowClusterNum;
	uint32_t fileSize; // subdir == 0x0
} PACKED;

struct ExtDirEntry
{

};

struct PathInfo
{
	uint32_t clusterNumber;
	bool isDir;
	uint32_t size;
};

PartitionInfo* partitions[4];



inline uint32_t clusterToLba(uint32_t clusterNumber, PartitionInfo* partitionInfo)
{
	return partitionInfo->dataLba + (clusterNumber * partitionInfo->clusterSize);
}


void* loadCluster(size_t num, void* dest, PartitionInfo* info, bool fromLba = false)
{
	uint64_t lba = num;
	if (!fromLba)
		lba = info->rootDirLba + (num * info->clusterSize);

	size_t clusterSize = info->clusterSize * info->sectorSize;
	for (size_t i = 0; i < clusterSize / 512; i++)
		ctx->loadSector(lba + i, dest + (i * 512));
	return dest;
}

bool findInRootDir(const char* str, size_t rootDirLba, PartitionInfo* info, PathInfo* pathInfo)
{
	void* entries = MM::alloc(info->clusterSize * info->sectorSize);
	loadCluster(rootDirLba, reinterpret_cast<void*>(entries), info, true);

	const size_t numOfEntries = info->clusterSize * info->sectorSize / 32;

	for (size_t i = 0; i < numOfEntries; i++)
	{
		DirEntry* entry = reinterpret_cast<DirEntry*>(entries + (i * 32));
		if (strncmp(entry->name, str, 11) == 0)
		{
			uint32_t clusterNum = entry->highClusterNum;
			clusterNum << 16;
			clusterNum |= entry->lowClusterNum;
			MM::free(entries);
			pathInfo->clusterNumber = clusterNum;
			pathInfo->isDir = entry->fileSize == 0;
			pathInfo->size = entry->fileSize;
			return true;
		}
	}

	// load all other root dir clusters



	pathInfo->clusterNumber = invalidClusterNum;
	pathInfo->isDir = false;

	MM::free(entries);
	return false;
}

void formatString(char* str)
{
	Ascii::toUpperCase(str);
	
	char buf[8];
	char ext[3];

	size_t extIndex = 0;
	bool foundExtension = false;

	for (size_t i = 0; i < strlen(str) && i < 11; i++)
	{
		if(str[i] == '.')
		{
			foundExtension = true;
			for(size_t j = i; j < 8; j++)
				buf[j] = ' ';
		}
		else if(foundExtension)
		{
			ext[extIndex++] = str[i];
			if(extIndex >= 3)
				break;
		}
		else
		{
			buf[i] = str[i];
		}
	}
	if(!foundExtension)
	{
		for(size_t i = strlen(str); i < 8; i++)
			buf[i] = ' ';
		for(size_t i = 0; i < 3; i++)
			ext[i] = ' ';
	}
	memcpy(str, buf, 8);
	memcpy(str + 8, ext, 3);
}

bool getPathInfo(char* str, PartitionInfo* partitionInfo, PathInfo* pathInfo)
{
	if (str[0] == '/')
		str++;

	if (Ascii::isNumeric(str[0]) && (str[1] == ':' || str[1] == '/'))
	{
		if (str[1] == ':')
		{
			if (str[2] == '/')
				str += 3;
			else
				str += 2;
		}
		else if (str[1] == '/')
		{
			str += 2;
		}
	}

	size_t targetLba = partitionInfo->rootDirLba;

	size_t bufIndex = 0;

	char buf[12];
	memset(buf, 0, 12);

	for (size_t i = 0; i < strlen(str); i++)
	{
		if (str[i] != '/')
			buf[bufIndex++] = str[i];
		else if (bufIndex != 0)
		{
			formatString(buf);

			// Vga::print("\n");
			// Vga::print(buf);

			if (!findInRootDir(buf, targetLba, partitionInfo, pathInfo))
				return false;

			targetLba = clusterToLba(pathInfo->clusterNumber, partitionInfo);

			// Vga::print("\nread lba");
			// Vga::print(utoa(targetLba, buf, 16));
			// Vga::print("\n");

			bufIndex = 0;
			memset(buf, 0, 12);
		}
	}

	if (bufIndex != 0)
	{
		formatString(buf);

		// Vga::print("\n");
		// Vga::print(buf);

		if (!findInRootDir(buf, targetLba, partitionInfo, pathInfo))
			return false;
	}

	return true;
}

PartitionInfo* findPartition(uint32_t lba)
{
	for (size_t i = 0; i < 4; i++)
		if ((partitions[i] != nullptr) && (partitions[i]->lba == lba))
			return partitions[i];
	return nullptr;
}

PartitionInfo* getPartitionInfo(uint32_t lba)
{
	PartitionInfo* pi = findPartition(lba);
	if (pi == nullptr)
	{
		pi = MM::alloc<PartitionInfo>(sizeof(PartitionInfo));
		pi->lba = lba;

		void* bpb = MM::alloc(512);
		ctx->loadSector(lba, bpb);
		uint16_t reservedSectors = *reinterpret_cast<uint16_t*>(bpb + bpbReservedSectorsOffset);
		pi->fatLba = lba + reservedSectors;
		pi->sectorSize = *reinterpret_cast<uint16_t*>(bpb + bpbSectorSizeOffset);
		pi->clusterSize = *reinterpret_cast<uint8_t*>(bpb + bpbClusterSizeOffset);
		pi->sectorsPerFat = *reinterpret_cast<uint32_t*>(bpb + bpbSectorsPerFatOffset);
		pi->numberOfFats = *reinterpret_cast<uint8_t*>(bpb + bpbNumberOfFats);
		pi->rootDirLba = pi->fatLba + (pi->numberOfFats * pi->sectorsPerFat);
		pi->dataLba = pi->rootDirLba - (*reinterpret_cast<uint32_t*>(bpb + bpbRootDirClusterOffset) * pi->clusterSize); // ... ???

		MM::free(bpb);
	}
	return pi;
}

bool canDrive(Drive::PartitionEntry* entry)
{
	void* bpb = MM::alloc(512);
	ctx->loadSector(entry->lba, bpb);
	char* fsInfoStr = reinterpret_cast<char*>(bpb + bpbFsInfoStrOffset);
	bool canDrive = strncmp(fsInfoStr, "FAT32   ", bpbFsInfoStrSize) == 0;
	MM::free(bpb);
	return canDrive;
}

size_t getFileSize(Drive::PartitionEntry* e, const char* path)
{
	Vga::print("[FAT32] get file size: ");
	Vga::print(path);
	Vga::print("\n");

	return 0;
}

size_t getNextClusterNumber(uint32_t clusterNum)
{

}

void* loadFile(Drive::PartitionEntry* e, const char* path, void* dest)
{
	PartitionInfo* info = getPartitionInfo(e->lba);

	char buf[16];
	Vga::print("\n---- fat32 ----\nloading file: ");
	Vga::print(path);
	Vga::print("\npartition lba: ");
	Vga::print(utoa(info->lba, buf, 16));
	Vga::print("\ncluster size: ");
	Vga::print(utoa(info->clusterSize, buf, 10));
	Vga::print(" sectors\nsector size: ");
	Vga::print(utoa(info->sectorSize, buf, 10));
	Vga::print(" bytes... \n");

	PathInfo pathInfo;

	if (getPathInfo(const_cast<char*>(path), info, &pathInfo))
	{
		char buf[16];
		Vga::print("\nfound at lba: ");
		Vga::print(utoa(clusterToLba(pathInfo.clusterNumber, info), buf, 16));
		Vga::print("\nsize: ");
		Vga::print(utoa(pathInfo.size, buf, 16));
		Vga::print("\n");
	}
	else
	{
		Vga::print("\nfile or directory does not exists!\n");
	}

	Vga::print("---- fat32 ----\n");

	return nullptr;
}

CORE_MODULE_LOAD(FAT32_LOAD_NAME, loadContext)
{
	FS::DriverInfo info = {
		.name = "FAT32",
		.loadFile = loadFile,
		.getFileSize = getFileSize,
		.canDrive = canDrive
	};
	ctx = loadContext->registerFS(&info);
	// memset(&partitions, 0, sizeof(PartitionInfo) * 4);
}

CORE_MODULE_UNLOAD(FAT32_UNLOAD_NAME, unloadContext)
{
	unloadContext->unregisterFS(ctx->id);
	for (size_t i = 0; i < 4; i++)
	{
		if (partitions[i] != nullptr)
			MM::free(partitions[i]);
	}
}
