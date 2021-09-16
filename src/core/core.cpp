#include "core/vga.hpp"
#include "core/keyboard.hpp"
#include "core/config.hpp"
#include "core/string.hpp"
#include "core/mm.hpp"
#include "core/ascii.hpp"

struct BootInfo
{
	uint32_t coreBinarySize;
	char *config;
	uint32_t configSize;
	uint32_t memMapSize;
	void *memMap;
} __attribute__((packed));

extern "C" void halt();

void printEntries(Config::Entry *entries, uint32_t entryCount);
uint32_t getBootOption(Config::Entry *entries, uint32_t entryCount);

void main()
{
	BootInfo *bootInfo = *(reinterpret_cast<BootInfo **>(0x3000 - 8));
	bootInfo->config[bootInfo->configSize] = '\0';

	VGA::init();
	VGA::cls();

	MM::init(bootInfo->memMap, bootInfo->memMapSize, bootInfo->coreBinarySize);

	halt();

	Config::parse(bootInfo->config);

	uint32_t entryCount = Config::entryCount();
	Config::Entry *entries = Config::entries();

	if (entryCount == 0)
	{
		VGA::print("No bootable configurations were found!\n");
		VGA::print("starting shell...");
	}
	else
	{
		printEntries(entries, entryCount);

		uint32_t num = getBootOption(entries, entryCount);

		if (num == 0)
		{
			VGA::print("starting shell...");
		}
		else
		{
			VGA::printChar(static_cast<char>(num + 48));
			VGA::print("\n\n");
			VGA::print("booting ");

			const Config::Entry *e = &entries[num - 1];

			const uint32_t nameSize = e->nameSize + 1;
			const uint32_t pathSize = e->pathSize + 1;

			char name[nameSize];
			char path[pathSize];

			memcpy(name, e->name, e->nameSize);
			name[nameSize] = '\0';

			memcpy(path, e->path, e->pathSize);
			path[pathSize] = '\0';

			VGA::print(name);
			VGA::print(" (");
			VGA::print(path);
			VGA::print(")\n");
		}
	}

	halt();
}

void printEntries(Config::Entry *entries, uint32_t entryCount)
{
	for (size_t i = 0; i < entryCount; i++)
	{
		VGA::printChar('[');
		char buf[5];
		utoa(i + 1, buf, 10);
		VGA::print(buf);
		VGA::print("] ");

		const uint32_t nl = entries[i].nameSize;
		const uint32_t pl = entries[i].pathSize;

		for (size_t j = 0; j < nl; j++)
			VGA::printChar(entries[i].name[j]);

		VGA::print(" (");

		for (size_t j = 0; j < pl; j++)
			VGA::printChar(entries[i].path[j]);

		VGA::print(")\n");
	}
}

uint32_t getBootOption(Config::Entry *entries, uint32_t entryCount)
{
	uint32_t num = 0xFFFFFFFF;

	char buf[8];

	while (num > entryCount)
	{
		VGA::print("\nChoose a number to boot or shell: ");
		Keyboard::getLine(buf, 8);
		num = Ascii::strToInt(buf);
		if (num > entryCount)
		{
			if (strcmp(buf, "shell") == 0)
			{
				return 0;
			}
			else
			{
				VGA::print(buf);
				VGA::print(" is an invalid boot option!");
			}
		}
	}

	return num;
}
