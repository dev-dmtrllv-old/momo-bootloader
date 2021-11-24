#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"

namespace ISR
{
	struct Entry
	{
		uint16_t lowAddr;
		uint16_t cs;
		uint8_t reserved;
		uint8_t attributes;
		uint16_t highAddr;
	} PACKED;

	struct IDTR
	{
		uint16_t limit;
		uint32_t base;
	} PACKED;

	struct Registers
	{
		uint32_t ds;                                     // Data segment selector
		uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
		uint32_t int_no, err_code;                       // Interrupt number and error code (if applicable)
		uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
	} PACKED;

	void init();
};
