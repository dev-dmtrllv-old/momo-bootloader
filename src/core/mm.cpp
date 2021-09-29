#include "core/mm.hpp"
#include "core/bios.hpp"
#include "core/core_info.hpp"
#include "core/macros.hpp"

extern void writeLine(const char*);

namespace MM
{
	namespace
	{
		struct BiosMemoryEntry
		{
			uint64_t base;
			uint64_t size;
			uint32_t usedFlag;
			uint32_t acpiFlag;
		} PACKED;

		BiosMemoryEntry* buf = nullptr;
	};

	void init()
	{
		if (buf == nullptr)
		{
			auto bufAddr = align(CoreInfo::getBiosEndAddr(), 0x10);
			buf = reinterpret_cast<BiosMemoryEntry*>(bufAddr);

			Bios::Registers regs = {};
			regs.di = bufAddr;

			call_bios_routine(&bios_get_mem_map, &regs);

			for (size_t i = 0; i < regs.cx; i++)
			{
				if(buf[i].usedFlag == 1)
				{
					writeLine("free");
				}
				else
				{
					writeLine("used");
				}
			}
		}
	}
};
