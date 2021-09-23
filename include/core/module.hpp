#pragma once
#include "core/fs.hpp"

namespace Module
{
	struct LoadContext
	{
		FS::RegisterFunc registerFS;
	};

	struct UnloadContext
	{
		FS::UnregisterFunc unregisterFS;
	};

#ifdef MOMO_MODULE
	
#else
	void addCoreModule();

#endif
};

#ifdef MOMO_MODULE
#define MODULE_LOAD(paramName) extern "C" void momoModuleLoad(const Module::LoadContext* (paramName))
#define MODULE_UNLOAD(paramName) extern "C" void momoModuleUnload(const Module::UnloadContext* (paramName))
#endif
