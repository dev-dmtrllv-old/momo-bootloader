#pragma once

#include "core/types.hpp"
#include "core/drive.hpp"

namespace FS
{
	typedef void *(*LoadSectorFunc)(uint64_t, void *);
	typedef void* (*LoadFileFunc)(Drive::PartitionEntry* entry, const char* path, void* dest);
	typedef size_t (*FileSizeFunc)(Drive::PartitionEntry* entry, const char* path);
	typedef bool (*CanDriveFunc)(Drive::PartitionEntry* entry);

	struct Context
	{
		uint32_t id;
		LoadSectorFunc loadSector;
		Drive::PartitionTable* partitionTable;
	};

	struct DriverInfo
	{
		const char* name;
		LoadFileFunc loadFile = nullptr;
		FileSizeFunc getFileSize = nullptr;
		CanDriveFunc canDrive = nullptr;
	};

	typedef Context* (*RegisterFunc)(FS::DriverInfo* info);
	typedef void (*UnregisterFunc)(uint32_t id);

	void init();

	Context* registerDriver(FS::DriverInfo* info);
	void unregisterDriver(const uint32_t id);

	DriverInfo* getDriver(const char* str);

	size_t getFileSize(const char* path);
	void* loadFile(const char* path, void* dest);
};
