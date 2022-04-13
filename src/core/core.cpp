#include "core/bios.hpp"
#include "core/mm.hpp"
#include "core/vesa.hpp"
#include "core/string.hpp"
#include "core/macros.hpp"
#include "core/disk.hpp"
#include "core/fs.hpp"
#include "core/shell.hpp"
#include "core/list.hpp"
#include "core/module.hpp"
#include "core/path.hpp"

void main(uint16_t bootDriveNumber)
{
	// Vesa::clear();

	INFO("initializing disk driver...");
	Disk::init(bootDriveNumber);

	INFO("initializing filesystems...");
	FS::init();

	char buf[128];
	memset(buf, 0, 128);
	Path::resolve(128, buf, "0:/", "momo", "modules", "../", "../");
	INFO(buf);

	INFO("initializing shell...");
	Shell::init();

	Module::initSystem();
	// Module::loadModule("ext2");

	INFO("starting shell");
	Shell::start();
}
