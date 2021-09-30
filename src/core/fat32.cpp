#include "core/fat32.hpp"

#include "core/disk.hpp"
#include "core/vesa.hpp"
#include "core/string.hpp"
#include "core/mm.hpp"
#include "core/ascii.hpp"

namespace FS
{
	namespace BPBOffsets
	{
		constexpr uint16_t bytesPerSector = 0xB;
		constexpr uint16_t sectorsPerCluster = 0xD;
		constexpr uint16_t reservedSectors = 0xE;
		constexpr uint16_t numberOfFats = 0x10;
		constexpr uint16_t sectorsPerFat = 0x24;
		constexpr uint16_t rootDirCluster = 0x2C;
		constexpr uint16_t fsType = 0x52;
	};

	constexpr uint32_t invalidClusterNum = 0xFFFFFFFF;
	constexpr uint32_t endOfCluster = 0x0FFFFFF8;

	uint32_t Fat32::clusterToLba(uint32_t clusterNumber)
	{
		return dataLba_ + (clusterNumber * clusterSize_);
	}

	uint32_t Fat32::lbaToCluster(uint32_t lba)
	{
		return (lba - dataLba_) / clusterSize_;
	}

	void Fat32::init(uint32_t lba)
	{
		INFO("initializing Fat-32 Driver...");

		void* bpbBuffer = MM::getPage();

		Disk::readSectors(bpbBuffer, lba, 1);

		BPBInfo bpbInfo;

		bpbInfo.bytesPerSector = *reinterpret_cast<uint16_t*>(bpbBuffer + BPBOffsets::bytesPerSector);
		bpbInfo.sectorsPerCluster = *reinterpret_cast<uint8_t*>(bpbBuffer + BPBOffsets::sectorsPerCluster);
		bpbInfo.reservedSectors = *reinterpret_cast<uint16_t*>(bpbBuffer + BPBOffsets::reservedSectors);
		bpbInfo.numberOfFats = *reinterpret_cast<uint8_t*>(bpbBuffer + BPBOffsets::numberOfFats);
		bpbInfo.sectorsPerFat = *reinterpret_cast<uint32_t*>(bpbBuffer + BPBOffsets::sectorsPerFat);
		bpbInfo.rootDirCluster = *reinterpret_cast<uint32_t*>(bpbBuffer + BPBOffsets::rootDirCluster);

		this->sectorSize_ = bpbInfo.bytesPerSector;
		this->clusterSize_ = bpbInfo.sectorsPerCluster;
		this->fatLba_ = lba + bpbInfo.reservedSectors;
		this->rootDirLba_ = this->fatLba_ + (bpbInfo.numberOfFats * bpbInfo.sectorsPerFat);
		this->dataLba_ = this->rootDirLba_ - (bpbInfo.rootDirCluster * this->clusterSize_);

		MM::freePage(bpbBuffer);
	}


	uint32_t Fat32::getNextCluster(uint32_t clusterNumber)
	{
		const uint16_t i = this->sectorSize_ / 4;
		const uint16_t fatLba = (clusterNumber / i) + this->fatLba_;

		void* buf = MM::getPage();
		Disk::readSectors(buf, fatLba, 1);

		const uint16_t offset = clusterNumber % i;
		const uint32_t newClusterNumber = *reinterpret_cast<uint32_t*>(buf + offset);

		MM::freePage(buf);

		return newClusterNumber;
	}

	void Fat32::formatPath(char* path)
	{
		Ascii::toUpperCase(path);

		char buf[8];
		char ext[3];

		size_t extIndex = 0;
		bool foundExtension = false;

		for (size_t i = 0; i < strlen(path) && i < 11; i++)
		{
			if (path[i] == '.')
			{
				foundExtension = true;
				for (size_t j = i; j < 8; j++)
					buf[j] = ' ';
			}
			else if (foundExtension)
			{
				ext[extIndex++] = path[i];
				if (extIndex >= 3)
					break;
			}
			else
			{
				buf[i] = path[i];
			}
		}
		if (!foundExtension)
		{
			for (size_t i = strlen(path); i < 8; i++)
				buf[i] = ' ';
			for (size_t i = 0; i < 3; i++)
				ext[i] = ' ';
		}
		memcpy(path, buf, 8);
		memcpy(path + 8, ext, 3);
	}

	void* Fat32::loadCluster(uint32_t lba, void* dest)
	{
		size_t clusterSize = this->clusterSize_ * this->sectorSize_;
		for (size_t i = 0; i < clusterSize / 512; i++)
			Disk::readSectors(dest + (i * 512), lba + i, 1);
		return dest;
	}

	bool Fat32::findInRootDir(const char* str, uint32_t clusterNum, EntryInfo* entryInfo)
	{
		void* entries = MM::getPage();

		const size_t numOfEntries = this->clusterSize_ * this->sectorSize_ / 32;

		// load all other root dir clusters...
		while (clusterNum < endOfCluster)
		{

			loadCluster(clusterToLba(clusterNum), reinterpret_cast<void*>(entries));

			for (size_t i = 0; i < numOfEntries; i++)
			{
				RootDirEntry* entry = reinterpret_cast<RootDirEntry*>(entries + (i * 32));

				if (strncmp(entry->name, str, 11) == 0)
				{
					uint32_t c = (entry->highClusterNum << 16) | entry->lowClusterNum;
					entryInfo->clusterNumber = c;
					entryInfo->isDir = entry->fileSize == 0;
					entryInfo->size = entry->fileSize;
					MM::freePage(entries);
					return true;
				}
			}

			clusterNum = getNextCluster(clusterNum);
		}

		MM::freePage(entries);

		entryInfo->clusterNumber = invalidClusterNum;
		entryInfo->isDir = false;

		return false;
	}

	bool Fat32::getEntryInfo(EntryInfo* entry, const char* path)
	{
		char* p = const_cast<char*>(path);

		char pathBuf[16];
		size_t pathBufIndex = 0;
		memset(pathBuf, 0, 16);

		uint32_t targetCluster = lbaToCluster(this->rootDirLba_);

		while (*p != '\0')
		{
			if (*p == '/')
			{
				if (pathBufIndex > 0)
				{
					pathBuf[pathBufIndex] = '\0';

					this->formatPath(pathBuf);

					if (!findInRootDir(pathBuf, targetCluster, entry))
						return false;

					targetCluster = entry->clusterNumber;

					pathBufIndex = 0;
					memset(pathBuf, 0, 16);
				}
			}
			else if (pathBufIndex < 16)
			{
				pathBuf[pathBufIndex++] = *p;
			}

			p++;
		}

		if (pathBufIndex != 0)
		{
			this->formatPath(pathBuf);

			if (!findInRootDir(pathBuf, targetCluster, entry))
				return false;
		}

		return true;
	}

	bool Fat32::getPathInfo(FS::PathInfo* info, const char* path)
	{
		EntryInfo entry;
		if (!getEntryInfo(&entry, path))
		{
			return false;
		}
		info->isDirectory = entry.isDir;
		info->size = entry.size;
		return true;
	}

	void Fat32::readFile(const char* path, void* dest)
	{
		EntryInfo entry;
		
		if (!getEntryInfo(&entry, path))
			return;

		uint32_t destAddr = reinterpret_cast<uint32_t>(dest);
		uint32_t c = entry.clusterNumber;
		
		while(c < endOfCluster)
		{
			this->loadCluster(clusterToLba(c), reinterpret_cast<void*>(destAddr));
			destAddr += this->clusterSize_ * this->sectorSize_;
			c = getNextCluster(c);
		}
	}

	void Fat32::readDir(const char* path, void* dest, size_t maxItems)
	{

	}
};