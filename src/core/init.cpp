// #include "core/vga.hpp"
#include "core/macros.hpp"

extern "C" void start () SECTION(".text.entry");

extern void main(uint16_t bootDriveNumber);
extern "C" void initGlobalCtors();
extern "C" void finiGlobalCtors();

[[noreturn]] inline void halt() { __asm__ volatile("cli\nhlt"); halt(); }

void start ()
{
	uint16_t bootDriveNumber = *reinterpret_cast<uint16_t*>(0x1000);

	initGlobalCtors();
	main(bootDriveNumber);
	// Vga::print("\n[core exit]\n");
	finiGlobalCtors();
	// Vga::print("[halted]\n");
	halt();
}
