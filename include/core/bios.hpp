#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"

namespace Bios
{
	struct Registers
	{
		uint32_t eax;
		uint32_t ebx;
		uint32_t ecx;
		uint32_t edx;
		uint32_t esi;
		uint32_t edi;
	};
};

extern "C" void call_bios_routine(void (* routine)(), Bios::Registers* registers);
extern "C" void bios_test();
