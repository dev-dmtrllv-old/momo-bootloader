#include "core/boot.hpp"
#include "core/string.hpp"

namespace Boot
{
	const Boot::Info* getInfo()
	{
		return *(reinterpret_cast<Boot::Info**>(0x3000 - 8));
	}

};
