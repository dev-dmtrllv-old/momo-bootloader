#include "core/drive.hpp"
#include "core/bios.hpp"
#include "core/mm.hpp"
#include "core/string.hpp"
#include "core/vga.hpp"
#include "core/macros.hpp"

namespace Drive
{
	namespace
	{
		struct Dap
		{
			uint8_t size;
			uint8_t reserved;
			uint16_t sectors;
			uint16_t bufferOffset;
			uint16_t bufferSegment;
			uint64_t lba;
		} PACKED;

		Dap* dap_;
		PartitionTable* pt_ = nullptr;

		uint32_t bootDriveNumber_;
		uint32_t sectorSize_;

		void* tempFileBuffer_;
	};

	void init(uint32_t bootDriveNumber, uint32_t sectorSize)
	{
		bootDriveNumber_ = bootDriveNumber;
		sectorSize_ = sectorSize;
		tempFileBuffer_ = MM::reserve(0x0, 0xFFFFF, sectorSize);
		dap_ = MM::alloc<Dap>(sizeof(Dap));
		dap_->size = sizeof(Dap);
		dap_->reserved = 0;
		dap_->sectors = 1;
		dap_->bufferSegment = static_cast<uint16_t>(reinterpret_cast<uint32_t>(tempFileBuffer_) >> 16);
		dap_->bufferOffset = static_cast<uint16_t>(reinterpret_cast<uint32_t>(tempFileBuffer_) & 0xFFFF);
		dap_->lba = 0;

	}

	[[noreturn]] inline void halt() { __asm__ volatile("cli\nhlt"); }

	void* loadSector(uint64_t lba, void* dest)
	{
		dap_->lba = lba;
		Bios::Registers regs = {
			.eax = 0x4200,
			.edx = bootDriveNumber_,
			.esi = reinterpret_cast<uint32_t>(dap_),
		};
		BIOS_INTERRUPT($0x13, regs);
		return memcpy(dest, tempFileBuffer_, sectorSize_);
	}

	PartitionTable* getPartitionTable()
	{
		if (pt_ == nullptr)
		{
			void* tempBuf = MM::alloc(sectorSize_);
			loadSector(0, tempBuf);
			memcpy(pt_, tempBuf + 0x01BE, sizeof(PartitionTable));
			MM::free(tempBuf);
		}
		return pt_;
	}
};
