#include "core/vesa.hpp"
#include "core/macros.hpp"
#include "core/mm.hpp"

extern "C" void start () SECTION(".text.entry");

extern void main(uint16_t bootDriveNumber);
extern "C" void initGlobalCtors();
extern "C" void finiGlobalCtors();

[[noreturn]] inline void halt() { __asm__ volatile("cli\nhlt"); halt(); }

void start ()
{
	uint16_t bootDriveNumber = *reinterpret_cast<uint16_t*>(0x1000);

	Vesa::init();

	MM::init();

	initGlobalCtors();

	INFO("main entry");
	main(bootDriveNumber);
	INFO("main exit");

	finiGlobalCtors();

	INFO("halted");
	halt();
}
