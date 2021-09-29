#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"

namespace Bios
{
	struct alignas(8) Registers
	{
		uint16_t ax;
		uint16_t bx;
		uint16_t cx;
		uint16_t dx;
		uint16_t di;
		uint16_t si;
	} PACKED;
};

extern "C" void call_bios_routine(void (* routine)(), Bios::Registers* registers);
extern "C" void bios_test();
extern "C" void bios_read_sectors();
