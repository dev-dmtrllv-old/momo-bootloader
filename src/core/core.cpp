#include "core/bios.hpp"
#include "core/mm.hpp"
#include "core/vesa.hpp"

#include "core/string.hpp"

#include "core/macros.hpp"

void main()
{
	using Color = Vesa::Color;

	Vesa::init();
	Vesa::clear();
	
	INFO("core entry");

	INFO("initializing memory manager...");
	
	MM::init();
}
