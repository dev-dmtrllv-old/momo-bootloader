#pragma once

#include "core/types.hpp"

extern const void* __coreBinaryStart;
extern const void* __coreBinaryEnd;

namespace CoreInfo
{
	inline uint32_t getBinaryStartAddr() { return reinterpret_cast<uint32_t>(&__coreBinaryStart); };
	inline uint32_t getBinaryEndAddr() { return reinterpret_cast<uint32_t>(&__coreBinaryEnd); };
};
