#include "core/ascii.hpp"
#include "core/string.hpp"

namespace Ascii
{
	bool isNumeric(char c) { return c >= 48 && c <= 57; }
	bool isLetter(char c) { return (c >= 65 && c <= 90) || (c >= 97 && c <= 122); }
	bool isAlphaNumeric(char c) { return isNumeric(c) || isLetter(c); }
	bool isChar(char c) { return c >= 32 && c <= 126; }
	bool isControlChar(char c) { return !isChar(c) && c < 127; }
	bool checkControlChar(char c, ControlChar controlChar) { return c == static_cast<char>(controlChar); }
	uint32_t toInt(char c) { return c - 48; };

	uint32_t pow(uint32_t x, uint32_t y)
	{
		if (y == 0)
			return 1;
		else if (y % 2 == 0)
			return pow(x, y / 2) * pow(x, y / 2);
		else
			return x * pow(x, y / 2) * pow(x, y / 2);
	}

	uint32_t strToInt(char *str)
	{
		const size_t l = strlen(str);
		size_t off = 0;
		uint32_t total = 0;
		for (size_t i = l; i > 0; i--)
			total += pow(10, off++) * toInt(str[i - 1]);
		return total;
	}
};
