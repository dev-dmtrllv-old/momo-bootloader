#include "core/module.hpp"
#include "core/list.hpp"
#include "core/elf.hpp"

namespace Module
{
	namespace
	{
		bool isInitialized_ = false;
		List<Module::Context> modules_;
		Module::LoadContext loadCtx_;
		Module::UnloadContext unloadCtx_;
	};

	void initSystem()
	{
		if (!isInitialized_)
		{
			loadCtx_ = {
				.registerFsDriver = FS::registerDriver,
			};

			unloadCtx_ = {
				.unregisterFsDriver = FS::unregisterDriver,
			};

			isInitialized_ = true;
		}
		else
		{
			ERROR("Module system is already initialized!");
		}
	}

	Module::Context* loadModule(const char* name)
	{
		if (isInitialized_)
		{
			Vesa::writeLine("loading module ", name, "...");
			Module::Context* ctx = modules_.find([name](const Module::Context& ctx, const size_t i) { return strcmp(ctx.name, name) == 0; });
			if (ctx == nullptr)
			{
				const char* modulesPath = "0:/momo/modules/";
				const size_t modulesPathLength = strlen(modulesPath);
				const size_t nameLength = strlen(name);

				char pathBuffer[64];

				memcpy(pathBuffer, modulesPath, modulesPathLength);
				memcpy(pathBuffer + modulesPathLength, name, nameLength);
				memcpy(pathBuffer + modulesPathLength + nameLength, ".mod", 5);

				FS::PathInfo pi;
				if (!FS::getPathInfo(&pi, pathBuffer))
				{
					Vesa::writeLine("file does not exists!");
				}
				else
				{
					if (pi.isDirectory)
					{

					}
					else
					{
						size_t filePages = 0;
						void* fileBuffer = MM::getPages(pi.size, &filePages);
						FS::readFile(pathBuffer, fileBuffer);
						Vesa::writeLine("module \"", pathBuffer, "\" loaded at ", utoa(reinterpret_cast<uint32_t>(fileBuffer), INT_STR_BUFFER, 16));
						if(Elf::isValid(fileBuffer))
						{
							Vesa::writeLine("Elf header validated!");
							Elf::info(fileBuffer);
							Elf::load(fileBuffer, pi.size);
						}
						else
						{
							ERROR("INVALID ELF MODULE!");
						}
					}
				}
			}
		}
		else
		{
			ERROR("Module system is not initialized yet!");
		}
	}

	void unloadModule(const char* name)
	{
		if(isInitialized_)
		{
			size_t index = 0;

		Module::Context* ctx = modules_.find([&](const Module::Context& ctx, const size_t i) {
			if (strcmp(ctx.name, name) == 0) {
				index = i;
				return true;
			}
			return false;
		});

		if (ctx != nullptr)
		{
			modules_.remove(index);
			ctx->unload(unloadCtx_);
		}
		}
		else
		{
			ERROR("Module system is not initialized yet");
		}
	}
};
