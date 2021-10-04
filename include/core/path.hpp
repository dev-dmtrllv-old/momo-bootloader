#pragma once

#include "core/types.hpp"
#include "core/string.hpp"
#include "core/macros.hpp"

namespace Path
{
	static const char* trim(const char* path, size_t* size)
	{
		while (*path == '/')
			path++;

		// INFO(path);

		if (path[0] == '\0')
		{
			*size = 0;
		}
		else
		{
			size_t i = strlen(path) - 1;
			while (path[i] == '/')
				i--;

			// char buf[17];
			// ERROR(utoa(i, buf, 10));

			*size = i + 1;
		}
		return path;
	}

	template<typename A, typename B>
	static bool resolve(size_t maxLen, A a, B b)
	{
		size_t al = strlen(a);

		if (al >= maxLen)
		{
			a[maxLen - 1] = '\0';
			return false;
		}
		else if (al == 0)
		{
			size_t i = 0;
			while (al < maxLen && i < strlen(b))
			{
				a[al++] = b[i++];
			}
			if (al >= maxLen)
			{
				WARN("al too big");
				return false;
			}
		}
		else
		{
			if (a[al - 1] != '/')
			{
				if (al + 1 >= maxLen)
				{
					a[maxLen - 1] = '\0';
					return false;
				}
				a[al++] = '/';
			}

			size_t bl = 0;
			b = trim(b, &bl);


			if (strncmp(b, "..", 2) == 0)
			{
				// INFO("back...")
					size_t i = al - 2;
				while (i > 0)
				{
					if (a[i--] == '/')
						break;
				}
				if (i != 0)
				{
					if (a[i] == ':')
						a[++i] = '/';

					a[i] = '\0';

				}

			}
			else if(!(bl == 1 && b[0] == '.'))
			{
				size_t i = 0;
				while (al < maxLen && i < bl)
				{
					a[al++] = b[i++];
				}
				if (al >= maxLen)
					return false;
			}
		}
		return true;
	}

	template<typename A, typename B, typename ...Ts>
	void resolve(size_t maxLen, A buf, B p, Ts... rest)
	{
		if (resolve(maxLen, buf, p))
			resolve(maxLen, buf, rest...);
	};
};
