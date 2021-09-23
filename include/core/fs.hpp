#pragma once

#include "core/types.hpp"

namespace FS
{
	typedef void *(*LoadSectorFunc)(uint64_t, void *);

	struct Context
	{
		LoadSectorFunc loadSector;
	};

	void registerDriver();
};
