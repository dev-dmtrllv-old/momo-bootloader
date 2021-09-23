#include "core/fs.hpp"
#include "core/list.hpp"
#include "core/drive.hpp"
#include "core/vga.hpp"
#include "core/string.hpp"

namespace FS
{
	namespace
	{
		struct DriverNode
		{
			FS::DriverInfo info;
			FS::Context ctx;
		};

		List<DriverNode> drivers;
	};

	Context* registerDriver(const FS::DriverInfo* const info)
	{
		static uint32_t driverCounter = 0;
		DriverNode n = { .info = *info, .ctx = { .id = driverCounter++, .loadSector = Drive::loadSector } };
		return &drivers.add(n)->ctx;
	}

	void test()
	{
		drivers.forEach([](const DriverNode* n, size_t i) {
			char buf[16];
			Vga::print(n->info.name);
			Vga::print(utoa(n->ctx.id, buf, 10));
			Vga::print("\n");
		});
	}
};
