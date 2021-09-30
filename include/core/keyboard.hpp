#pragma once

#include "core/types.hpp"

namespace Keyboard
{
	struct KeyInfo
	{
		uint8_t keyCode; // the key number
		char charCode; // the ascii character code
	};

	void getChar(Keyboard::KeyInfo* info);
};
