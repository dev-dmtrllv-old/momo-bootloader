#include "core/vga.hpp"

namespace VGA
{
	namespace
	{
		int cursor_pos = 0;
	};

	void printChar(char c)
	{
		volatile unsigned char *mem = reinterpret_cast<unsigned char *>(0xb8000);
		mem[cursor_pos++ * 2] = c;
	}

	void print(const char *str)
	{
		while (*str != 0)
		{
			printChar(*str);
			str++;
		}
	}

	void cls()
	{
		volatile unsigned char *mem = reinterpret_cast<unsigned char *>(0xb8000);
		for (unsigned int i = 0; i < 80 * 16; i++)
			mem[i * 2] = ' ';

		cursor_pos = 0;
	}

};
