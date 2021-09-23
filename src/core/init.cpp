extern "C" void start () __attribute__((section(".text.entry")));

extern void main();
extern "C" void initGlobalCtors();
extern "C" void finiGlobalCtors();

[[noreturn]] inline void halt() { __asm__ volatile("cli\nhlt"); halt(); }

extern "C" void start ()
{
	initGlobalCtors();
	main();
	finiGlobalCtors();
	halt();
}
