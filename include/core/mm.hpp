#pragma once

#include "core/types.hpp"

namespace MM
{
	constexpr uint32_t pageSize = 0x1000;

	void init();

	template<typename T>
	constexpr T align(T n, size_t alignment = pageSize)
	{
		if (n % alignment == 0)
			return n;
		return ((n / alignment) + 1) * alignment;
	}

	template<typename T>
	constexpr T alignDown(T n, size_t alignment = pageSize)
	{
		if (n % alignment == 0)
			return n;
		return (n / alignment) * alignment;
	}
	
	void* getPage();
	void freePage(void* address);
};
