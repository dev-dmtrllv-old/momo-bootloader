#include "core/fs.hpp"
#include "core/list.hpp"
#include "core/drive.hpp"
#include "core/vga.hpp"
#include "core/string.hpp"
#include "core/ascii.hpp"

namespace FS
{
	namespace
	{
		struct DriverNode
		{
			FS::DriverInfo info;
			FS::Context ctx;
		};

		List<DriverNode> drivers_;

		FS::DriverInfo* partitions_[4];

		void* loadFileNotImplemented(const char* path, void* dest) { return nullptr; }
		size_t getFileSizeNotImplemented(const char* path) { return 0; }
		bool canDriveNotImplemented(Drive::PartitionEntry* e) { return false; }

		uint32_t driverCounter_;

		void createNewContext(FS::Context* ctx)
		{
			ctx->id = driverCounter_++;
			ctx->loadSector = Drive::loadSector;
			ctx->partitionTable = Drive::getPartitionTable();
		}

		bool parsePartitionIndex(const char* path, size_t* indexPtr)
		{
			char indexChar = path[0] == '/' ? path[1] : path[0];
			size_t i = Ascii::toInt(path[1]);

			if (i > 3)
				return false;

			*indexPtr = i;

			return true;
		}

		FS::DriverInfo* getDriverForPartition(const size_t partitionIndex)
		{
			if (partitions_[partitionIndex] == nullptr)
			{
				Vga::print("searching for a fs driver...\n");
				const FS::DriverNode* node = drivers_.find([&](const DriverNode* n, size_t i) {
					return n->info.canDrive(&Drive::getPartitionTable()->entries[partitionIndex]);
				});

				if (node != nullptr)
					partitions_[partitionIndex] = const_cast<FS::DriverInfo*>(&node->info);
				else
					Vga::print("fs not found...\n");
			}

			return partitions_[partitionIndex];
		}
	};

	void init()
	{
		driverCounter_ = 0;
		partitions_[0] = nullptr;
		partitions_[1] = nullptr;
		partitions_[2] = nullptr;
		partitions_[3] = nullptr;

	}

	Context* registerDriver(FS::DriverInfo* info)
	{
		const DriverInfo* foundDriver = getDriver(info->name);
		if (foundDriver != nullptr)
		{
			// ERROR
			return nullptr;
		}

		DriverNode n = { .info = *info };

		createNewContext(&n.ctx);

		if (n.info.getFileSize == nullptr)
			n.info.getFileSize = getFileSizeNotImplemented;

		if (n.info.loadFile == nullptr)
			n.info.loadFile = loadFileNotImplemented;

		if (n.info.canDrive == nullptr)
			n.info.canDrive = canDriveNotImplemented;

		return &drivers_.add(n)->ctx;
	}

	DriverInfo* getDriver(const char* str)
	{
		const DriverNode* foundNode = drivers_.find([&](const DriverNode* n, size_t i) {
			return strcmp(const_cast<char*>(n->info.name), const_cast<char*>(str)) == 0;
		});
		if (foundNode != nullptr)
			return const_cast<DriverInfo*>(&foundNode->info);
		return nullptr;
	}

	size_t getFileSize(const char* path)
	{
		size_t ptIndex = 0;
		if (parsePartitionIndex(path, &ptIndex))
		{
			FS::DriverInfo* driver = getDriverForPartition(ptIndex);
			if(driver != nullptr)
				return driver->getFileSize(path[0] == '/' ? &path[3] : &path[2]);
		}

		return 0;
	}

	void* loadFile(const char* path, void* dest)
	{
		size_t ptIndex = 0;
		if (parsePartitionIndex(path, &ptIndex))
		{
			FS::DriverInfo* driver = getDriverForPartition(ptIndex);
			if(driver != nullptr)
				return driver->loadFile(path[0] == '/' ? &path[3] : &path[2], dest);
		}
		return nullptr;
	}
};
