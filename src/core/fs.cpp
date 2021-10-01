#include "core/fs.hpp"

#include "core/mm.hpp"
#include "core/string.hpp"
#include "core/macros.hpp"
#include "core/fat32.hpp"
#include "core/disk.hpp"
#include "core/ascii.hpp"

namespace FS
{
	namespace
	{
		bool isInitialized_ = false;
		FS::Driver* partitions_[4];

		FS::Fat32 rootFs;
	};

	void init()
	{
		if (!isInitialized_)
		{
			memset(partitions_, 0, sizeof(FS::Driver*) * 4);

			const Disk::PartitionTable* const pt = Disk::getPartitionTable();
			rootFs.initDriver(pt->partitions[0].lba);
			partitions_[0] = &rootFs;
		}
		else
		{
			WARN("FS is already initialized!");
		}
	}

	void loadDriver(size_t partitionIndex, FS::Driver* driver, size_t driverSize)
	{
		if (!isInitialized_)
		{
			WARN("FS is not initialized yet!");
			return;
		}
		else if (partitionIndex > 4)
		{
			ERROR("Cannot load more than 4 drivers!");
		}
		else if (partitionIndex == 0)
		{
			ERROR("Cannot change root partition driver!");
		}

		const Disk::PartitionTable* const pt = Disk::getPartitionTable();
		driver->initDriver(pt->partitions[partitionIndex].lba);
		partitions_[partitionIndex] = driver;
	}


	bool getPathInfo(FS::PathInfo* pathInfo, const char* path)
	{
		if (path[0] == '/' || path[0] == ':')
			path++;

		uint16_t partitionIndex = Ascii::toInt(path[0]);

		FS::Driver* d = partitions_[partitionIndex];
		if (d == nullptr)
		{
			ERROR("NO DRIVER INSTALLED FOR PARTITION");
			return false;
		}

		return d->getPathInfo(pathInfo, &path[2]);
	}

	bool readFile(const char* path, void* file)
	{
		if (path[0] == '/' || path[0] == ':')
			path++;

		uint16_t partitionIndex = Ascii::toInt(path[0]);

		FS::Driver* d = partitions_[partitionIndex];
		if (d == nullptr)
		{
			ERROR("NO DRIVER INSTALLED FOR PARTITION");
			return false;
		}

		return d->readFile(&path[2], file);
	}

	void readDir(const char* path, ReadDirCallback callback)
	{
		if (path[0] == '/' || path[0] == ':')
			path++;

		uint16_t partitionIndex = Ascii::toInt(path[0]);

		FS::Driver* d = partitions_[partitionIndex];
		if (d == nullptr)
		{
			ERROR("NO DRIVER INSTALLED FOR PARTITION");
			return;
		}

		return d->readDir(&path[2], callback);
	}
};
