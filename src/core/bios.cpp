#include "core/bios.hpp"

namespace Bios
{
	void call(Bios::Routine routine, Bios::Registers* regs)
	{
		call_bios_routine(routine, regs);
		
	}
};
