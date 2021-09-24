#include "core/fat32.hpp"
#include "core/module.hpp"
#include "core/vga.hpp"
#include "core/mm.hpp"
#include "core/string.hpp"

constexpr uint32_t fsInfoStrOffset = 0x52;
constexpr size_t fsInfoStrSize = 8;

FS::Context* ctx;

bool canDrive(Drive::PartitionEntry* entry)
{
	void* bpb = MM::alloc(512);
	ctx->loadSector(entry->lba, bpb);
	char* fsInfoStr = reinterpret_cast<char*>(bpb + fsInfoStrOffset);
	bool canDrive = strncmp(fsInfoStr, "FAT32   ", fsInfoStrSize) == 0;
	MM::free(bpb);
	return canDrive;
}

size_t getFileSize(const char* path)
{
	Vga::print("[FAT32] get file size: ");
	Vga::print(path);
	Vga::print("\n");

	return 0;
}

void* loadFile(const char* path, void* dest)
{
	Vga::print("[FAT32] load file: ");
	Vga::print(path);
	Vga::print("\n");



	return nullptr;
}

CORE_MODULE_LOAD(FAT32_LOAD_NAME, loadContext)
{
	FS::DriverInfo info = {
		.name = "FAT32",
		.loadFile = loadFile,
		.getFileSize = getFileSize,
		.canDrive = canDrive
	};
	ctx = loadContext->registerFS(&info);
}

CORE_MODULE_UNLOAD(FAT32_UNLOAD_NAME, unloadContext)
{
	unloadContext->unregisterFS(ctx->id);
}
