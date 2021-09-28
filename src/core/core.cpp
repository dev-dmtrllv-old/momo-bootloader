#include "core/bios.hpp"

void write(const char* str)
{
	unsigned char* vgaMem = reinterpret_cast<unsigned char*>(0xB8000);
	while(*str != '\0')
	{
		*vgaMem++ = *str++;
		vgaMem++;
	}
}

void main()
{
	write("Hello, World!");
	Bios::Registers regs = {
		.eax = 0,
		.ebx = 0,
		.ecx = 0,
		.edx = 0,
		.esi = 0,
		.edi = 0,
	};
	call_bios_routine(&bios_test, &regs);

	write("...done!");
}
