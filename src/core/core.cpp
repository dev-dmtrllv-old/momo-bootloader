#include "core/bios.hpp"
#include "core/mm.hpp"

void write(const char* str)
{
	Bios::Registers regs = {};
	call_bios_routine(&bios_get_cursor_position, &regs);
	
	uint8_t row = Bios::higherReg(regs.dx);
	uint8_t col = Bios::lowerReg(regs.dx);

	uint16_t offset = (row * 80) + col;
	
	unsigned char* vgaMem = reinterpret_cast<unsigned char*>(0xB8000 + (offset * 2));
	while (*str != '\0')
	{
		if(*str == '\n')
		{
			offset = ((offset / 80) + 1) * 80;
			vgaMem = reinterpret_cast<unsigned char*>(0xB8000 + (offset * 2));
			*str++;
		}
		else
		{
			*vgaMem++ = *str++;
			vgaMem++;
			offset++;
		}
	}

	regs.bx = 0;
	regs.dx = Bios::combineReg(offset % 80, offset / 80);

	call_bios_routine(&bios_set_cursor_position, &regs);
}

void writeLine(const char* str)
{
	write(str);
	write("\n");
}

void main()
{
	writeLine("Hello, World!");

	MM::init();

	writeLine("done");
}
