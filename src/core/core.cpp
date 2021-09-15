#include "core/vga.hpp"
#include "core/keyboard.hpp"
#include "core/config.hpp"
#include "core/string.hpp"
#include "core/mm.hpp"

struct BootInfo
{
	uint32_t coreBinarySize;
	char *config;
	uint32_t configSize;
	uint32_t memMapSize;
	void* memMap;
} __attribute__((packed));

extern "C" void halt();

void main()
{
	using namespace VGA;
	using namespace Keyboard;

	BootInfo *bootInfo = *(reinterpret_cast<BootInfo **>(0x5000 - 8));
	bootInfo->config[bootInfo->configSize] = '\0';

	cls();

	// char b[16];
	// utoa((uint32_t)bootInfo->memMap, b, 16);
	// print(b);

	// VGA::init();

	mm::init(bootInfo->memMap, bootInfo->memMapSize, bootInfo->coreBinarySize);

	// Config::parse(bootInfo->config);

	// uint32_t entryCount = Config::entryCount();
	// Config::Entry *entries = Config::entries();

	// for (size_t i = 0; i < entryCount; i++)
	// {
	// 	VGA::printChar('[');
	// 	char buf[5];
	// 	utoa(i + 1, buf, 10);
	// 	VGA::print(buf);
	// 	VGA::print("] ");

	// 	const uint32_t nl = entries[i].nameSize;
	// 	const uint32_t pl = entries[i].pathSize;

	// 	for (size_t j = 0; j < nl; j++)
	// 		VGA::printChar(entries[i].name[j]);

	// 	VGA::print(" (");

	// 	for (size_t j = 0; j < pl; j++)
	// 		VGA::printChar(entries[i].path[j]);

	// 	VGA::print(")");

	// 	const uint8_t spaces = 80 - (pl + nl + 7);
	// 	for (size_t j = 0; j < spaces; j++)
	// 		VGA::printChar(' ');

	// }

	halt();

	while (true)
	{
	};
}
