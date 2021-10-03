#pragma once

#include "core/fs.hpp"
#include "core/macros.hpp"

namespace Module
{
	// define module api for registering different types of modules
	struct LoadContext
	{
		FS::RegisterDriver registerFsDriver;
	} PACKED;

	// module api's but for unregistering
	struct UnloadContext
	{
		FS::UnregisterDriver unregisterFsDriver;
	} PACKED;

#ifdef MOMO_MODULE
	#define MODULE_LOAD(paramName) extern "C" void momoModuleLoad(const Module::LoadContext& paramName)
	#define MODULE_UNLOAD(paramName) extern "C" void momoModuleUnload(const Module::UnloadContext& paramName)
#else
	typedef void(*LoadModuleFunc)(const Module::LoadContext&);
	typedef void(*UnloadModuleFunc)(const Module::UnloadContext&);

	struct Context
	{
		const char* name;
		LoadModuleFunc load;
		UnloadModuleFunc unload;
	} PACKED;

	void initSystem();
	Module::Context* loadModule(const char* name);
	void unloadModule(const char* name);

	#define CORE_MODULE_LOAD(funcName, paramName) extern void (funcName)(const Module::LoadContext& paramName)
	#define CORE_MODULE_UNLOAD(funcName, paramName) extern void (funcName)(const Module::UnloadContext& paramName)

#endif
};
