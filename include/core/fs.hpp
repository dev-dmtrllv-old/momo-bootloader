#pragma once

#include "core/types.hpp"

namespace FS
{
	typedef void *(*LoadSectorFunc)(uint64_t, void *);

	struct Context
	{
		uint32_t id;
		LoadSectorFunc loadSector;
	};

	struct DriverInfo
	{
		const char* name;
	};

	typedef Context* (*RegisterFunc)(const FS::DriverInfo* const info);
	typedef void (*UnregisterFunc)(uint32_t id);

	void test();

	Context* registerDriver(const FS::DriverInfo* const info);
	void unregisterDriver(const uint32_t id);
};
