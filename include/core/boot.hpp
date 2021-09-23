#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"

namespace Boot
{
	struct Info
	{
		uint32_t bootDriveNumber;
		uint32_t sectorSize;
		uint32_t coreBinarySize;
		char* config;
		uint32_t configSize;
		uint32_t memMapSize;
		void* memMap;
	} PACKED;

	const Boot::Info* getInfo();
};
