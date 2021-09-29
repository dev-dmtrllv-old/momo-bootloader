#include "core/bios.hpp"
#include "core/mm.hpp"
#include "core/vesa.hpp"
#include "core/string.hpp"
#include "core/macros.hpp"
#include "core/disk.hpp"

void main(uint16_t bootDriveNumber)
{
	Vesa::init();
	Vesa::clear();

	INFO("core entry");

	INFO("initializing memory manager...");
	MM::init();

	INFO("initializing disk driver...");
	Disk::init(bootDriveNumber);

	const Disk::PartitionTable* const pt = Disk::getPartitionTable();

	char buf[16];

	for (size_t i = 0; i < 4; i++)
	{
		utoa(pt->partitions[i].lba, buf, 16);
		INFO(buf);
	}
}
