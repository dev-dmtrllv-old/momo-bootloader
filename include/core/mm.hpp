#pragma once

#include "core/types.hpp"

namespace mm
{
	struct MemBlock
	{
		uint64_t base;
		uint64_t size;
		uint32_t acpi_flags;
		uint32_t misc_flags;
	} __attribute__((packed));

	void init(void * memMap, uint32_t size);
};
