#pragma once

#include "core/vesa.hpp"

#define PACKED __attribute__((packed))
#define SECTION(symbol) __attribute__((section(symbol)))
#define UNUSED __attribute__((unused))

#ifndef INFO_COLOR
#define INFO_COLOR Vesa::Color::CYAN
#endif
#ifndef WARN_COLOR
#define WARN_COLOR Vesa::Color::MAGENTA
#endif
#ifndef ERROR_COLOR
#define ERROR_COLOR Vesa::Color::LIGHT_RED
#endif

#define INFO(msg) Vesa::write("[INFO] ", INFO_COLOR, Vesa::Color::BLACK);\
Vesa::writeLine(msg);

#define WARN(msg) Vesa::write("[WARN] ", WARN_COLOR, Vesa::Color::BLACK);\
Vesa::writeLine(msg);

#define ERROR(msg) Vesa::write("[ERROR] ", ERROR_COLOR, Vesa::Color::BLACK);\
Vesa::writeLine(msg);

#define INT_STR_BUFFER "\0\0\0\0\0\0\0\0\0\0"
