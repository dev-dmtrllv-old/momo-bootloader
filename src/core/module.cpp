#include "core/module.hpp"

#ifndef MOMO_MODULE

#include "core/list.hpp"

namespace Module
{

	namespace
	{
		List<Module::Handlers> coreModules;

		Module::LoadContext loadCtx = {
			.registerFS = FS::registerDriver
		};
	};

	void addCoreModule(Module::LoadFunc loader, Module::UnloadFunc unloader)
	{
		coreModules.add({ loader, unloader });
	}

	void loadCoreModules()
	{
		coreModules.forEach([&](const Module::Handlers* h, size_t i)
		{
			h->loader(&loadCtx);
		});
	}

	void unloadCoreModules()
	{

	};

};

#endif
