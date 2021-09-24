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
	typedef void (*LoadFunc)(const Module::LoadContext*);
	typedef void (*UnloadFunc)(const Module::UnloadContext*);

	struct Handlers
	{
		LoadFunc loader;
		UnloadFunc unloader;
	};

	struct Context
	{
		Handlers handlers;
	};

	void addCoreModule(LoadFunc loader, UnloadFunc unloader);

	void loadCoreModules();
#endif
};

#ifdef MOMO_MODULE
#define MODULE_LOAD(paramName) extern "C" void momoModuleLoad(const Module::LoadContext* (paramName))
#define MODULE_UNLOAD(paramName) extern "C" void momoModuleUnload(const Module::UnloadContext* (paramName))
#else
#define CORE_MODULE_LOAD(funcName, paramName) extern void (funcName)(const Module::LoadContext* (paramName))
#define CORE_MODULE_UNLOAD(funcName, paramName) extern void (funcName)(const Module::UnloadContext* (paramName))
#endif
