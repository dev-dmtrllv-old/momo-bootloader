#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"

#define BIOS_ROUTINE(routineName) extern "C" void routineName()

namespace Bios
{
	typedef void (*Routine)();

	struct alignas(8) Registers
	{
		volatile uint16_t ax;
		volatile uint16_t bx;
		volatile uint16_t cx;
		volatile uint16_t dx;
		volatile uint16_t di;
		volatile uint16_t si;
	} PACKED;


	constexpr uint8_t lowerReg(uint16_t reg) { return reg & 0xFF; };
	constexpr uint8_t higherReg(uint16_t reg) { return reg >> 8; };
	constexpr uint16_t combineReg(uint8_t low, uint8_t high) { return (high << 8) | low; }

	void call(Bios::Routine routine, Bios::Registers* regs);
};

extern "C" void call_bios_routine(Bios::Routine, Bios::Registers*);

BIOS_ROUTINE(bios_read_sectors);
BIOS_ROUTINE(bios_get_video_mode);
BIOS_ROUTINE(bios_set_cursor_position);
BIOS_ROUTINE(bios_get_cursor_position);
BIOS_ROUTINE(bios_get_mem_map);
BIOS_ROUTINE(bios_get_keyboard_char);
BIOS_ROUTINE(bios_shutdown);
