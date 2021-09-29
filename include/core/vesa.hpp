#pragma once

#include "core/types.hpp"

namespace Vesa
{
	void init();
	void write(const char*);
	void writeLine(const char*);
	uint16_t writeAt(const char* str, uint16_t offset);
	uint16_t writeAt(const char*, uint16_t row, uint16_t column);
	void clear();
}
