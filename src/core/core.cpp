#include "core/vga.hpp"
#include "core/keyboard.hpp"
#include "core/config.hpp"
#include "core/string.hpp"
#include "core/mm.hpp"
#include "core/ascii.hpp"
#include "core/vesa.hpp"
#include "core/drive.hpp"

struct BootInfo
{
	uint32_t bootDriveNumber;
	uint32_t sectorSize;
	uint32_t coreBinarySize;
	char *config;
	uint32_t configSize;
	uint32_t memMapSize;
	void *memMap;
} __attribute__((packed));

[[noreturn]] inline void halt() { __asm__ volatile("cli\nhlt"); }

void printEntries(Config::Entry *entries, uint32_t entryCount);
uint32_t getBootOption(uint32_t entryCount);

static BootInfo bootInfoCopy;

inline BootInfo *getBootInfo()
{
	const BootInfo *const b = *(reinterpret_cast<BootInfo **>(0x3000 - 8));
	return reinterpret_cast<BootInfo *>(memcpy(&bootInfoCopy, b, sizeof(BootInfo)));
}

void main()
{
	const BootInfo *const bootInfo = getBootInfo();
	bootInfo->config[bootInfo->configSize] = '\0';

	Vga::init();
	Vga::cls();

	MM::init(bootInfo->memMap, bootInfo->memMapSize);

	Drive::init(bootInfo->bootDriveNumber, bootInfo->sectorSize);

	Drive::PartitionTable pt;

	Drive::getPartitionTable(&pt);

	char buf[32];

	for(size_t i = 0; i < 4; i++)
	{
		Vga::print("partition ");
		Vga::print(utoa(i + 1, buf, 10));
		Vga::print(" flags:");
		Vga::print(utoa(pt.entries[i].flags, buf, 16));
		Vga::print("\n");
	}

	halt();

	if (!Vesa::init())
	{
		Vga::print("Vesa initialization error!");
		halt();
	}

	uint16_t foundMode = Vesa::findClosestMode(1920, 1080, 32, true);

	if (!Vesa::setMode(foundMode | 0x4000))
	{
		Vga::print("Could not change vesa mode!");
		halt();
	}

	Config::parse(bootInfo->config);

	uint32_t entryCount = Config::entryCount();
	Config::Entry *entries = Config::entries();

	if (entryCount == 0)
	{
		// Vga::print("No bootable configurations were found!\n");
		// Vga::print("starting shell...");
	}
	else
	{
		// printEntries(entries, entryCount);

		// uint32_t num = getBootOption(entryCount);

		// if (num == 0)
		// {
		// Vga::print("starting shell...");
		// }
		// else
		// {
		// Vga::print("\nbooting ");

		// const Config::Entry *e = &entries[num - 1];

		// const uint32_t nameSize = e->nameSize + 1;
		// const uint32_t pathSize = e->pathSize + 1;

		// char name[nameSize];
		// char path[pathSize];

		// memcpy(name, e->name, e->nameSize);
		// name[nameSize] = '\0';

		// memcpy(path, e->path, e->pathSize);
		// path[pathSize] = '\0';

		// Vga::print(name);
		// Vga::print(" (");
		// Vga::print(path);
		// Vga::print(")\n");
		// }
	}

	halt();
}

// void printEntries(Config::Entry *entries, uint32_t entryCount)
// {
// 	for (size_t i = 0; i < entryCount; i++)
// 	{
// 		Vga::printChar('[');
// 		char buf[5];
// 		utoa(i + 1, buf, 10);
// 		Vga::print(buf);
// 		Vga::print("] ");

// 		const uint32_t nl = entries[i].nameSize;
// 		const uint32_t pl = entries[i].pathSize;

// 		for (size_t j = 0; j < nl; j++)
// 			Vga::printChar(entries[i].name[j]);

// 		Vga::print(" (");

// 		for (size_t j = 0; j < pl; j++)
// 			Vga::printChar(entries[i].path[j]);

// 		Vga::print(")\n");
// 	}
// }

// uint32_t getBootOption(uint32_t entryCount)
// {
// 	uint32_t num = 0xFFFFFFFF;

// 	char buf[128];

// 	char shellString[] = "shell";

// 	while (num > entryCount)
// 	{
// 		Vga::print("\nChoose a number to boot or shell: ");
// 		Keyboard::getLine(buf, 128);
// 		if (strcmp(shellString, buf) == 0)
// 		{
// 			return 0;
// 		}

// 		num = Ascii::strToInt(buf);

// 		if (num > entryCount || num == 0)
// 		{
// 			Vga::print(buf);
// 			Vga::print(" is an invalid boot option!");
// 			num = 0xFFFFFFFF;
// 		}
// 	}

// 	return num;
// }

// used when a pure virtual function cannot be be called
extern "C" void __cxa_pure_virtual() {  }
