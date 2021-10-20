#pragma once

#include "core/types.hpp"

extern const void* __coreBinaryStart;
extern const void* __coreBinaryEnd;

extern const void* __biosBinaryStart;
extern const void* __biosBinaryEnd;

namespace CoreInfo
{
	inline uint32_t getBinaryStartAddr() { return reinterpret_cast<uint32_t>(&__coreBinaryStart); };
	inline uint32_t getBinaryEndAddr() { return reinterpret_cast<uint32_t>(&__coreBinaryEnd); };

	inline uint32_t getBiosStartAddr() { return reinterpret_cast<uint32_t>(&__biosBinaryEnd); };
	inline uint32_t getBiosEndAddr() { return reinterpret_cast<uint32_t>(&__biosBinaryEnd); };
};
