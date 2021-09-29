#include "core/bios.hpp"

void write(const char* str)
{
	unsigned char* vgaMem = reinterpret_cast<unsigned char*>(0xB8000);
	while (*str != '\0')
	{
		*vgaMem++ = *str++;
		vgaMem++;
	}
}

void main()
{
	write("Hello, World!");

	// ; ax = lba low
	// ; bx = lba high
	// ; si = buffer address
	// ; di = buffer segment
	// ; cx = number of sectors
	// ; dx = drive number
	Bios::Registers regs = {
		.ax = 0x00,
		.bx = 0x00,
		.cx = 0x1,
		.dx = 0x80,
		.di = 0x0,
		.si = 0x5000,
	};

	// call_bios_routine(reinterpret_cast<void(*)()>(0x12345678), reinterpret_cast<Bios::Registers*>(0x66666666));
	call_bios_routine(&bios_read_sectors, &regs);

	write("done");
}
