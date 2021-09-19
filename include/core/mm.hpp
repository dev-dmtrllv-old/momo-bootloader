#pragma once

#include "core/types.hpp"

namespace MM
{
	void init(void *memMap, uint32_t size);
	void *reserve(uint32_t minAddr, uint32_t maxaDDR, uint32_t size);
	void *alloc(uint32_t size);
	void free(void *addr);

	template <typename T>
	T *alloc(uint32_t size) { return reinterpret_cast<T *>(alloc(size)); };
};
