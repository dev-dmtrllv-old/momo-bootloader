#include "core/bios.hpp"
#include "core/mm.hpp"
#include "core/vesa.hpp"
#include "core/string.hpp"
#include "core/macros.hpp"
#include "core/disk.hpp"
#include "core/fs.hpp"
#include "core/shell.hpp"

void main(uint16_t bootDriveNumber)
{
	Vesa::init();
	Vesa::clear();

	INFO("core entry");

	INFO("initializing memory manager...");
	MM::init();

	INFO("initializing disk driver...");
	Disk::init(bootDriveNumber);

	INFO("initializing filesystems...");
	FS::init();

	INFO("initializing shell...");
	Shell::init();

	INFO("starting shell");
	Shell::start();
}
