#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"

namespace Drive
{
	struct PartitionEntry
	{
		uint8_t flags;			// 0x80 = bootable
		uint8_t startHead;
		uint8_t startSector; 	// bit 6 - 7 are the 2 upper bits for startCylinder
		uint8_t startCylinder;
		uint8_t systemID;	
		uint8_t endHead;
		uint8_t endSector; 		// bit 6 - 7 are the 2 upper bits for endCylinder
		uint8_t endCylinder;	
		uint32_t lba;			// starting LBA of the partition
		uint32_t sectorCount; 	// number of sectors in the partition
	} PACKED;

	struct PartitionTable
	{
		PartitionEntry entries[4];
	} PACKED;

	void init(uint32_t bootDriveNumber, uint32_t sectorSize);

	void getPartitionTable(PartitionTable* pt);
};
