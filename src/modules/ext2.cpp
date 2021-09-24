#define MOMO_MODULE

#include "core/module.hpp"

FS::Context* fsCtx;

MODULE_LOAD(ctx)
{
	FS::DriverInfo info = {
		.name = "EXT2"
	};
	fsCtx = ctx->registerFS(&info);
}

MODULE_UNLOAD(ctx)
{
	ctx->unregisterFS(fsCtx->id);
}
