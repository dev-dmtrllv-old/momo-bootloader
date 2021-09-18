#pragma once

#include "core/types.hpp"

#define BIOS_INTERRUPT(interruptNumber, regs) __asm__ volatile(                                                                          \
	"int " #interruptNumber "\n"                                                                                                         \
	"mov %%eax, %[a]\n"                                                                                                                  \
	"mov %%ebx, %[b]\n"                                                                                                                  \
	"mov %%ecx, %[c]\n"                                                                                                                  \
	"mov %%edx, %[d]\n"                                                                                                                  \
	"mov %%edi, %[D]\n"                                                                                                                  \
	"mov %%esi, %[S]\n"                                                                                                                  \
	: [a] "=a"((regs).eax), [b] "=b"((regs).ebx), [c] "=c"((regs).ecx), [d] "=d"((regs).edx), [D] "=D"((regs).edi), [S] "=S"((regs).esi) \
	: "D"((regs).edi), "a"((regs).eax), "b"((regs).ebx), "c"((regs).ecx), "d"((regs).edx), "D"((regs).edi), "S"((regs).esi)              \
	:)

namespace Bios
{
	struct Registers
	{
		uint32_t eax = 0;
		uint32_t ebx = 0;
		uint32_t ecx = 0;
		uint32_t edx = 0;
		uint32_t esi = 0;
		uint32_t edi = 0;
	};
}
