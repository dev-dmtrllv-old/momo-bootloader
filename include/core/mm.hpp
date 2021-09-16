#pragma once

#include "core/types.hpp"

namespace MM
{
	void init(void * memMap, uint32_t size);
	void* alloc(uint32_t size);
	void free(void* addr);
};
