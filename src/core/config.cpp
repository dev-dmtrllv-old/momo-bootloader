#include "core/config.hpp"
#include "core/types.hpp"
#include "core/vga.hpp"
#include "core/ascii.hpp"

namespace Config
{
	namespace 
	{
		uint32_t entryCount_;

		Entry entries_[16];
	};

	uint32_t entryCount() { return entryCount_; }
	Entry* entries() { return entries_; }

	void parse(const char *str)
	{
		entryCount_ = 0;

		const char *entriesKeyWord = "entries";
		uint8_t keyWordIndex = 0;

		bool scanningEntries = false;
		bool isReadingKey = true;

		while (*str != '\0')
		{
			const char c = *str;

			if (!scanningEntries)
			{
				if (c == entriesKeyWord[keyWordIndex])
				{
					keyWordIndex++;
					if (keyWordIndex >= 7)
					{
						scanningEntries = true;
					}
				}
			}
			else if (isReadingKey)
			{
				if(c == ']')
					break;

				if (Ascii::isAlphaNumeric(c))
				{
					entries_[entryCount_].name = str;
					entries_[entryCount_].nameSize = 0;
					isReadingKey = false;
					while (*str != '\0') // search for end of the key
					{
						const char cc = *str;
						if (!Ascii::isAlphaNumeric(cc))
							break;
						entries_[entryCount_].nameSize++;
						str++;
					}
				}
			}
			else
			{
				if (c == '"')
				{
					str++;

					entries_[entryCount_].path = str;
					entries_[entryCount_].pathSize = 0;

					while (*str != '\0') // search for end of the path
					{
						const char cc = *str;
						str++;
						if (cc == '"')
							break;
						entries_[entryCount_].pathSize++;
					}
					isReadingKey = true;
					entryCount_++;
				}
			}

			str++;
		}
	}
};
