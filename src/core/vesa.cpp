#include "core/vesa.hpp"
#include "core/bios.hpp"
#include "core/string.hpp"

namespace Vesa
{
	namespace
	{
		typedef unsigned char* volatile VgaMem;

		constexpr uint32_t vgaMemAddr = 0xB8000;
		constexpr uint32_t vgaRows = 25;
		constexpr uint32_t vgaColumns = 80;

		constexpr uint32_t vgaMemAddrWithOffset(uint32_t offset)
		{
			return vgaMemAddr + (offset * 2);
		}

		constexpr uint32_t vgaMemAddrWithOffset(uint32_t row, uint32_t column)
		{
			return vgaMemAddr + (((row * vgaColumns) + column) * 2);
		}

		VgaMem getVgaMemAddr(uint32_t offset)
		{
			return reinterpret_cast<unsigned char*>(vgaMemAddrWithOffset(offset));
		}

		VgaMem getVgaMemAddr(uint32_t row, uint32_t column)
		{
			return reinterpret_cast<unsigned char*>(vgaMemAddrWithOffset(row, column));
		}

		uint16_t cursorOffset = 0;

		void setCursorPos(uint16_t row, uint16_t column)
		{
			cursorOffset = (row * vgaColumns) + column;
			Bios::Registers regs = {};
			regs.dx = Bios::combineReg(column, row);
			regs.bx = 0;
			call_bios_routine(&bios_set_cursor_position, &regs);
		}

		void setCursorPos(uint16_t offset)
		{
			setCursorPos(offset / vgaColumns, offset % vgaColumns);
		}

		void scroll()
		{
			for (size_t i = 0; i < vgaRows - 1; i++)
			{
				void* src = getVgaMemAddr(i + 1, 0);
				void* dest = getVgaMemAddr(i, 0);
				memcpy(dest, src, vgaColumns * 2);
			}

			VgaMem vgaMem = getVgaMemAddr(vgaRows - 1, 0);

			for (size_t i = 0; i < vgaColumns; i ++)
				vgaMem[i * 2] = ' ';
		}
	};

	void init()
	{
		Bios::Registers regs = {};
		call_bios_routine(&bios_get_cursor_position, &regs);

		uint8_t row = Bios::higherReg(regs.dx);
		uint8_t col = Bios::lowerReg(regs.dx);

		cursorOffset = (row * vgaColumns) + col;
	}

	void write(const char* str)
	{
		uint16_t off = writeAt(str, cursorOffset);
		setCursorPos(off);
	}

	void writeLine(const char* str)
	{
		uint16_t off = writeAt(str, cursorOffset);
		
		off = ((off / vgaColumns) + 1) * vgaColumns;
		
		if (off >= vgaColumns * vgaRows)
		{
			scroll();
			off = vgaColumns * (vgaRows - 1);
		}
		
		setCursorPos(off);
	}

	uint16_t writeAt(const char* str, uint16_t offset)
	{
		VgaMem vgaMem = getVgaMemAddr(offset);

		uint16_t newOffset = offset;

		while (*str != '\0')
		{
			if (*str == '\n')
			{
				offset = ((offset / vgaColumns) + 1) * vgaColumns;
				vgaMem = getVgaMemAddr(offset);
				*str++;
			}
			else if (*str == '\t')
			{
				const uint16_t of = offset % vgaColumns;
				uint8_t spaces = of % 4 == 0 ? 4 : (4 - (of % 4));
				for (uint8_t i = 0; i < spaces; i++)
				{
					*vgaMem++ = ' ';
					*vgaMem++;
				}
				*str++;
				offset += spaces;
			}
			else
			{
				*vgaMem++ = *str++;
				vgaMem++;
				newOffset++;
			}

			if (offset >= vgaColumns * vgaRows)
			{
				scroll();
				offset = vgaColumns * (vgaRows - 1);
				vgaMem = getVgaMemAddr(offset);
			}
		}

		return newOffset;
	}

	uint16_t writeAt(const char* str, uint16_t row, uint16_t column)
	{
		return writeAt(str, (row * vgaColumns) + column);
	}

	void clear()
	{
		for (size_t i = 0; i < vgaColumns * vgaRows; i++)
			writeAt(" ", i);
		setCursorPos(0);
	}
};
