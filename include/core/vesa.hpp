#pragma once

#include "core/types.hpp"

namespace Vesa
{
	enum class Color
	{
		BLACK = 0x0,
		BLUE = 0x1,
		GREEN = 0x2,
		CYAN = 0x3,
		RED = 0x4,
		MAGENTA = 0x5,
		BROWN = 0x6,
		LIGHT_GRAY = 0x7,
		DARK_GRAY = 0x8,
		LIGHT_BLUE = 0x9,
		LIGHT_GREEN = 0xA,
		LIGHT_CYAN = 0xB,
		LIGHT_RED = 0xC,
		LIGHT_MAGENTA = 0xD,
		YELLOW = 0xE,
		WHITE = 0xF,
	};

	void init();
	void init(Color foreground, Color background);

	void setColor(Color color);
	void setBackground(Color color);
	void setColors(Color foreground, Color background);

	uint16_t getCursorOffset();
	void setCursorOffset(uint16_t offset);
	void setCursorOffset(uint16_t row, uint16_t column);

	void write(const char* str);
	void writeLine(const char* str);
	uint16_t writeAt(const char* str, uint16_t offset);
	uint16_t writeAt(const char*, uint16_t row, uint16_t column);
	void write(const char* str, Color fg, Color bg);
	void writeLine(const char* str, Color fg, Color bg);

	void info(const char* str);
	void warn(const char* str);
	void error(const char* str);

	void clear();
}
