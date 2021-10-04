#pragma once

#include "core/types.hpp"

namespace Elf
{
	void info(void* elfPtr);
	bool isValid(void* elfPtr);
	bool load(void* buffer, size_t bufferSize);
};
