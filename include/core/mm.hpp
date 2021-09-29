#pragma once

#include "core/types.hpp"

namespace MM
{
	void init();	

	template<typename T>
	T align(T n, size_t alignment = 0x1000)
	{
		if(n % alignment == 0)
			return n;
		return ((n / alignment) + 1) * alignment;
	}
};
