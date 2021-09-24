#pragma once

#include "core/module.hpp"

#define FAT32_LOAD_NAME loadFat32Module
#define FAT32_UNLOAD_NAME unloadFat32Module

CORE_MODULE_LOAD(FAT32_LOAD_NAME, ctx);
CORE_MODULE_UNLOAD(FAT32_UNLOAD_NAME, ctx);
