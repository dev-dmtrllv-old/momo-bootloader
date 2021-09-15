#pragma once

#include "core/types.hpp"

namespace Config
{
	struct Entry
	{
		const char* name;
		uint32_t nameSize;
		const char* path;
		uint32_t pathSize;
	};


	uint32_t entryCount();
	Entry* entries();

	void parse(const char* str);
};
