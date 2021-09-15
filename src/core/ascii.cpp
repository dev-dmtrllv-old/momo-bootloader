#include "core/ascii.hpp"

namespace Ascii
{
	bool isAlphaNumeric(char c) { return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122); }
	bool isChar(char c) { return c >= 32 && c <= 126; }
	bool isControlChar(char c) { return !isChar(c) && c < 128; }
	bool checkControlChar(char c, ControlChar controlChar) { return c == static_cast<char>(controlChar); }
};
