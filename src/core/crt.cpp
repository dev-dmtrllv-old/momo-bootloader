#include "core/types.hpp"

typedef void (*CrtArrFunc[]) (void);

extern CrtArrFunc __preinit_array_start __attribute__((weak));
extern CrtArrFunc __preinit_array_end __attribute__((weak));
extern CrtArrFunc __init_array_start __attribute__((weak));
extern CrtArrFunc __init_array_end __attribute__((weak));
extern CrtArrFunc __fini_array_start __attribute__((weak));
extern CrtArrFunc __fini_array_end __attribute__((weak));

extern "C" void initGlobalCtors()
{
	size_t count = __preinit_array_end - __preinit_array_start;
	for (size_t i = 0; i < count; i++)
		__preinit_array_start[i]();

	count = __init_array_end - __init_array_start;
	for (size_t i = 0; i < count; i++)
		__init_array_start[i]();
}

extern "C" void finiGlobalCtors()
{
	size_t count = __fini_array_end - __fini_array_start;
	for (size_t i = count; i > 0; i--)
		__fini_array_start[i - 1]();
}

