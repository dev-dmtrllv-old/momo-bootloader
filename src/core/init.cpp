// #include "core/vga.hpp"
#include "core/macros.hpp"

extern "C" void start () SECTION(".text.entry");

extern void main();
extern "C" void initGlobalCtors();
extern "C" void finiGlobalCtors();

[[noreturn]] inline void halt() { __asm__ volatile("cli\nhlt"); halt(); }

void start ()
{
	initGlobalCtors();
	main();
	// Vga::print("\n[core exit]\n");
	finiGlobalCtors();
	// Vga::print("[halted]\n");
	halt();
}
