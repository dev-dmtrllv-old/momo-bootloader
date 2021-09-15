#include "core/vga.hpp"
#include "core/keyboard.hpp"
#include "core/config.hpp"
#include "core/string.hpp"

struct BootInfo
{
	char *config;
	unsigned int configSize;
	unsigned int memMapSize;
	void *memMap;
};

extern "C" void halt();

void main()
{
	using namespace VGA;
	using namespace Keyboard;

	BootInfo *bootInfo = *(reinterpret_cast<BootInfo **>(0x5000 - 4));
	bootInfo->config[bootInfo->configSize] = '\0';

	cls();

	Config::parse(bootInfo->config);

	uint32_t entryCount = Config::entryCount();
	Config::Entry *entries = Config::entries();

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

		VGA::print(")");


		const uint8_t spaces = 80 - (pl + nl + 7);
		for (size_t j = 0; j < spaces; j++)
			VGA::printChar(' ');

	}

	halt();

	while (true){};
}
