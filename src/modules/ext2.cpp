#define MOMO_MODULE

#include "core/module.hpp"

class Ext2 : public FS::Driver
{
public:
	Ext2() : FS::Driver("Ext2") {};

protected:
	void init(uint32_t lba) override
	{

	}

	bool canDrive(uint32_t lba) override
	{
		return false;
	}

	bool getPathInfo(FS::PathInfo* info, const char* path) override
	{
		return false;
	}

	bool readFile(const char* path, void* dest) override
	{
		return false;
	}

	void readDir(const char* path, void* dest, size_t maxItems) override
	{

	}

	void readDir(const char* path, FS::ReadDirCallback callback) override
	{

	}
};

static Ext2 ext2Driver = Ext2();

MODULE_LOAD(ctx)
{
	volatile unsigned char* vgaMem = reinterpret_cast<volatile unsigned char*>(0xB8000);
	unsigned int i = 0;
	unsigned int j = 0;
	while (i++)
	{
		if (i > (0xFFFFFFFF - 1))
			vgaMem[j += 2] = 'x';
	}

	ctx.registerFsDriver(&ext2Driver);
}

MODULE_UNLOAD(ctx)
{
	ctx.unregisterFsDriver(&ext2Driver);
}
