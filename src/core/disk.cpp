#include "core/disk.hpp"

#include "core/mm.hpp"
#include "core/macros.hpp"
#include "core/bios.hpp"
#include "core/string.hpp"

namespace Disk
{
	namespace
	{
		constexpr uint16_t dapSectorSize = 0x200;
		constexpr uint16_t partitionTableOffset = 0x01BE;

		bool isInitialized_ = false;
		uint16_t driveNumber_ = 0x80;
		Disk::PartitionTable partitionTable_;

		PartitionTable* loadPartitionTable(PartitionTable* table)
		{
			void* buf = MM::getPage();
			readSectors(buf, 0, 1);
			memcpy(table, static_cast<void*>(buf + partitionTableOffset), sizeof(PartitionTable));
			MM::freePage(buf);
		}
	};

	void init(uint16_t driveNumber)
	{
		if (!isInitialized_)
		{
			driveNumber_ = driveNumber;

			loadPartitionTable(&partitionTable_);

			isInitialized_ = true;
		}
		else
		{
			WARN("Fat-32 driver is already initialized!");
		}
	}

	void readSectors(void* dest, uint32_t lba, uint16_t sectors)
	{
		void* buf = MM::allocRealModeBuffer(dapSectorSize);
		uint32_t destAddr = reinterpret_cast<uint32_t>(dest);

		Bios::Registers regs;

		for (size_t i = 0; i < sectors; i++)
		{
			regs.ax = lba;
			regs.bx = lba >> 16;
			regs.cx = 1;
			regs.dx = driveNumber_;
			regs.di = 0;
			regs.si = reinterpret_cast<uint32_t>(buf);

			call_bios_routine(&bios_read_sectors, &regs);

			memcpy(reinterpret_cast<void*>(destAddr), buf, dapSectorSize);

			destAddr += dapSectorSize;
			lba++;
		}

		MM::freeRealModeBuffer(buf);
	}

	const PartitionTable* const getPartitionTable()
	{
		return &partitionTable_;
	}
};
