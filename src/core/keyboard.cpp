#include "core/keyboard.hpp"
#include "core/vga.hpp"

namespace Keyboard
{
	char getChar()
	{
		int c;
		asm volatile(
			"xor %%eax, %%eax;\n\t"
			"int $0x16;\n\t"
			"mov %%eax, %0"
			: "=r"(c)
			:
			: "eax");
		return static_cast<char>(c & 0xFF);
	}

	void getLine(char *buffer, unsigned int size)
	{
		char c = getChar();
		unsigned int i = 0;
		while (c != 13 && i < size)
		{
			if (c <= 126 && c >= 32)
			{
				VGA::printChar(c);
				*(buffer++) = c;
			}
			c = getChar();
		}
	}
};
