#include "core/path.hpp"


namespace Path
{
	bool isAbsolute(const char* path) { return Ascii::isNumeric(path[0]) && path[1] == ':'; }
};
