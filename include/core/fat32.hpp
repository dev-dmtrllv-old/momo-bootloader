#pragma once

#include "core/fs.hpp"
#include "core/macros.hpp"

namespace FS
{
	class Fat32 : public FS::Driver
	{
	private:
		struct BPBInfo
		{
			uint16_t bytesPerSector;
			uint8_t sectorsPerCluster;
			uint16_t reservedSectors;
			uint8_t numberOfFats;
			uint32_t sectorsPerFat;
			uint32_t rootDirCluster;
		} PACKED;

		struct RootDirEntry
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

	public:
		struct EntryInfo
		{
			uint32_t clusterNumber;
			uint32_t size;
			bool isDir;
		};

	private:
		uint32_t dataLba_;
		uint32_t rootDirLba_;
		uint32_t fatLba_;
		uint32_t clusterSize_; // size in sectors
		uint32_t sectorSize_; // size in bytes

		uint32_t clusterToLba(uint32_t clusterNumber);
		uint32_t lbaToCluster(uint32_t lba);
		void* loadCluster(size_t lba, void* dest);
		bool findInRootDir(const char* str, uint32_t rootDirLba, EntryInfo* entryInfo);
		void formatPath(char* path);

		bool getEntryInfo(EntryInfo* entry, const char* str);
		uint32_t getNextCluster(uint32_t clusterNumber);

	public:
		Fat32() : FS::Driver(), dataLba_(0), rootDirLba_(0), fatLba_(0), clusterSize_(0), sectorSize_(0) {};

		void init(uint32_t lba) override;

		bool getPathInfo(FS::PathInfo* info, const char* path) override;
		void readFile(const char* path, void* dest) override;
		void readDir(const char* path, void* dest, size_t maxItems) override;
	};
};
