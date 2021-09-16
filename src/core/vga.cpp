#include "core/vga.hpp"
#include "core/ascii.hpp"
#include "core/types.hpp"
#include "core/io.hpp"
#include "core/string.hpp"

namespace Vga
{
	namespace
	{
		constexpr uint8_t COLUMNS = 80;
		constexpr uint8_t ROWS = 25;
		constexpr uint32_t VGA_MEM_ADDR = 0xb8000;

		bool isInitialized_ = false;

		uint16_t cursorPos_ = 0;

		uint16_t getCursorOffset()
		{
			uint16_t pos = 0;
			IO::outb(0x3D4, 0x0F);
			pos |= IO::inb(0x3D5);
			IO::outb(0x3D4, 0x0E);
			pos |= ((uint16_t)IO::inb(0x3D5)) << 8;
			return pos;
		}

		void setCursorOffset(uint16_t offset)
		{
			IO::outb(0x3d4, 0x0f);
			IO::outb(0x3d5, (uint8_t)(offset & 0xff));
			IO::outb(0x3d4, 0x0e);
			IO::outb(0x3d5, (uint8_t)((offset >> 8) & 0xff));
		}

		void scroll()
		{
			volatile unsigned char *mem = reinterpret_cast<unsigned char *>(VGA_MEM_ADDR);

			for (size_t i = 0; i < ROWS - 1; i++)
				memcpy(reinterpret_cast<void *>(VGA_MEM_ADDR + ((i * COLUMNS)) * 2), reinterpret_cast<void *>(VGA_MEM_ADDR + (((i + 1) * COLUMNS) * 2)), sizeof(unsigned char) * COLUMNS);
			for (size_t i = 0; i < COLUMNS; i++)
				mem[((COLUMNS * (ROWS - 1)) * 2) + (i * 2)] = ' ';
		}
	};

	void init()
	{
		if (!isInitialized_)
		{
			cursorPos_ = getCursorOffset();
			isInitialized_ = true;
		}
	}

	void printChar(char c)
	{
		volatile unsigned char *mem = reinterpret_cast<unsigned char *>(VGA_MEM_ADDR);
		if (Ascii::checkControlChar(c, Ascii::ControlChar::LF))
		{
			cursorPos_ += COLUMNS - (cursorPos_ % COLUMNS);
			if (cursorPos_ >= COLUMNS * ROWS)
			{
				cursorPos_ = COLUMNS * (ROWS - 1);
				scroll();
			}
			setCursorOffset(cursorPos_);
		}
		else if (Ascii::isChar(c))
		{
			mem[cursorPos_++ * 2] = c;
			setCursorOffset(cursorPos_);
		}
	}

	void print(const char *str)
	{
		while (*str != 0)
		{
			printChar(*str);
			str++;
		}
	}

	void printLine(const char *str)
	{
		while (*str != 0)
		{
			printChar(*str);
			str++;
		}
		printChar('\n');
	}

	void cls()
	{
		volatile unsigned char *mem = reinterpret_cast<unsigned char *>(VGA_MEM_ADDR);
		for (unsigned int i = 0; i < COLUMNS * ROWS; i++)
			mem[i * 2] = ' ';

		cursorPos_ = 0;
		setCursorOffset(0);
	}
};
