#include "core/bios.hpp"
#include "core/mm.hpp"
#include "core/vesa.hpp"
#include "core/string.hpp"
#include "core/macros.hpp"
#include "core/disk.hpp"
#include "core/fs.hpp"

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

	FS::PathInfo pi;

	const char* path = "/0/momo/boot.cfg"; 

	if(!FS::getPathInfo(&pi, path))
	{
		ERROR("OOPS could not get path info!");
	}
	else
	{
		Vesa::write("Found file ");
		Vesa::write(path);
		Vesa::write(" of ");
		char buf[16];
		utoa(pi.size, buf, 10);
		Vesa::write(buf);
		Vesa::writeLine(" bytes...");
	
		void* file = MM::getPage();
		FS::readFile(path, file);
		utoa(reinterpret_cast<uint32_t>(file), buf, 16);
		Vesa::write("File buffer at address: ");
		Vesa::writeLine(buf);
	}
}
