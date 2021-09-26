#include "core/vga.hpp"
#include "core/keyboard.hpp"
#include "core/config.hpp"
#include "core/string.hpp"
#include "core/mm.hpp"
#include "core/ascii.hpp"
#include "core/vbe.hpp"
#include "core/drive.hpp"
#include "core/macros.hpp"
#include "core/boot.hpp"

void main()
{
	const Boot::Info* const bootInfo = Boot::getInfo();
	
	Vga::init();
	Vga::cls();

	// MM::init(bootInfo->memMap, bootInfo->memMapSize);

	// Drive::init(bootInfo->bootDriveNumber, bootInfo->sectorSize);

	// Drive::PartitionTable pt;

	// Drive::getPartitionTable(&pt);

	// char buf[32];

	// for (size_t i = 0; i < 4; i++)
	// {
	// 	Vga::print("partition ");
	// 	Vga::print(utoa(i + 1, buf, 10));
	// 	Vga::print(" flags:");
	// 	Vga::print(utoa(pt.entries[i].flags, buf, 16));
	// 	Vga::print("\n");
	// }

	return;
}
