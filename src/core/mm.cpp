#include "core/mm.hpp"

#include "core/string.hpp"
#include "core/vga.hpp"

namespace mm
{
	void init(void *memMap, uint32_t size)
	{
		VGA::print("init mm");

		MemBlock *m = reinterpret_cast<MemBlock *>(memMap);

		// char buf[16];
		// utoa((uint32_t)memMap, buf, 16);
		// VGA::print(buf);

		for (size_t i = 0; i < size; i++)
		{
			char buf[16];
			utoa(m[i].base, buf, 16);
			VGA::print(buf);
			VGA::print(" - ");
			utoa(m[i].size, buf, 16);
			VGA::print(buf);
			VGA::print("    ");
		}
	}
}
