#include "core/keyboard.hpp"

#include "core/bios.hpp"

namespace Keyboard
{
	void getChar(KeyInfo* info)
	{
		Bios::Registers regs = {};
		call_bios_routine(bios_get_keyboard_char, &regs);
		info->keyCode = regs.ax >> 8;
		info->charCode = regs.ax & 0xFF;
	}
};
