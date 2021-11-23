#include "core/elf.hpp"

#include "core/macros.hpp"
#include "core/vesa.hpp"
#include "core/string.hpp"
#include "core/types.hpp"
#include "core/list.hpp"

namespace Elf
{
	namespace
	{
		constexpr size_t identSize = 16;
		constexpr uint16_t getSymbolBindType(size_t t) { return t >> 4; }
		constexpr uint16_t getSymbolType(size_t t) { return t & 0x0F; }

		enum IdentIndex : uint16_t
		{
			II_MAG0 = 0,
			II_MAG1 = 1,
			II_MAG2 = 2,
			II_MAG3 = 3,
			II_CLASS = 4,
			II_DATA = 5,
			II_VERSION = 6,
			II_PAD = 7,
		};

		enum class Type : uint16_t
		{
			NONE = 0,	 		// No file type
			REL = 1,	 		// Relocatable file
			EXEC = 2,	 		// Executable file
			DYN = 3,	 		// Shared object file
			CORE = 4,			// Core file
			LOPROC = 0xff00,	// Processor-specific
			HIPROC = 0xffff,	// Processor-specific
		};

		enum class Machine : uint16_t
		{
			M_NONE = 0,
			M_M32 = 1,
			M_SPARC = 2,
			M_386 = 3,
			M_68K = 4,
			M_88K = 5,
			M_860 = 7,
			MIPS = 8
		};

		enum class ClassType : uint16_t
		{
			CLASS_NONE = 0,
			CLASS_32 = 1,
			CLASS_64 = 2
		};

		enum class DataType : uint16_t
		{
			CLASS_NONE = 0,
			DATA2LSB = 1,
			DATA2MSB = 2
		};

		enum SectionHeaderType : uint32_t
		{
			SHT_NULL = 0,   	// Null section
			SHT_PROGBITS = 1,   // Program information
			SHT_SYMTAB = 2,   	// Symbol table
			SHT_STRTAB = 3,   	// String table
			SHT_RELA = 4,   	// Relocation (w/ addend)
			SHT_NOBITS = 8,   	// Not present in file
			SHT_REL = 9,   		// Relocation (no addend)
		};

		enum class SectionHeaderAttribute : uint32_t
		{
			SHF_WRITE = 0x01,	// Writable section
			SHF_ALLOC = 0x02 	// Exists in memory
		};

		enum class SectionIndex : uint16_t
		{
			UNDEF = 0,
			LORESERVE = 0xff00,
			LOPROC = 0xff00,
			HIPROC = 0xff1f,
			ABS = 0xfff1,
			COMMON = 0xfff2,
			HIRESERVE = 0xffff,
		};

		struct Symbol
		{
			uint32_t name;
			uint32_t value;
			uint32_t size;
			uint8_t info;
			uint8_t other;
			SectionIndex shndx;
		} PACKED;

		enum SymbolBinging
		{
			STB_LOCAL = 0, // Local scope
			STB_GLOBAL = 1, // Global scope
			STB_WEAK = 2  // Weak, (ie. __attribute__((weak)))
		};

		enum class SymbolType
		{
			STT_NOTYPE = 0, // No type
			STT_OBJECT = 1, // Variables, arrays, etc.
			STT_FUNC = 2  // Methods or functions
		};

		struct Rel {
			uint32_t r_offset;
			uint32_t r_info;
		} PACKED;

		struct RelA {
			uint32_t offset;
			uint32_t info;
			int32_t	addend;
		} PACKED;

		struct Header
		{
			unsigned char ident[identSize];
			Type type;
			Machine machine;
			uint32_t version;
			uint32_t entry;
			uint32_t phoff;
			uint32_t shoff;
			uint32_t flags;
			uint16_t ehsize;
			uint16_t phentsize;
			uint16_t phnum;
			uint16_t shentsize;
			uint16_t shnum;
			uint16_t shstrndx;
		} PACKED;

		struct SectionHeader
		{
			uint32_t name;
			SectionHeaderType type;
			uint32_t flags;
			uint32_t addr;
			uint32_t offset;
			uint32_t size;
			uint32_t link;
			uint32_t info;
			uint32_t addrAlign;
			uint32_t entSize;
		};

		inline SectionHeader* getSectionHeader(Header* header)
		{
			return reinterpret_cast<SectionHeader*>(reinterpret_cast<uint32_t>(header) + header->shoff);
		}


		inline SectionHeader* getSection(Header* header, size_t index)
		{
			return &getSectionHeader(header)[index];
		}

		inline char* getStringTable(Header* header)
		{
			if (header->shstrndx)
				return reinterpret_cast<char*>(reinterpret_cast<uint32_t>(header) + getSection(header, header->shstrndx)->offset);
			return nullptr;
		}

		inline char* getString(Header* header, size_t offset)
		{
			char* strtab = getStringTable(header);
			if (strtab != nullptr)
				return strtab + offset;
			return nullptr;
		}

		void* lookupSymbol(const char* name)
		{
			return nullptr;
		}

		int getSymbolValue(Header* header, size_t tableIndex, size_t symbolIndex)
		{
			if (tableIndex == 0 || symbolIndex == 0)
				return 0;

			SectionHeader* sh = getSection(header, tableIndex);

			uint32_t entries = sh->size / sh->entSize;
			if (symbolIndex >= entries)
			{
				return -1;
			}

			int symAddr = reinterpret_cast<uint32_t>(header) + sh->offset;
			Symbol* symbol = &reinterpret_cast<Symbol*>(symAddr)[symbolIndex];
			if (symbol->shndx == SectionIndex::UNDEF)
			{
				// External symbol, lookup value
				SectionHeader* strtab = getSection(header, sh->link);
				const char* name = (const char*)header + strtab->offset + symbol->name;

				void* target = lookupSymbol(name);

				if (target == 0)
				{
					// Extern symbol not found
					if (getSymbolBindType(symbol->info) & STB_WEAK) // Weak symbol initialized as 0
						return 0;

					else // undefined symbol
						return -2;
				}
				else
				{
					return reinterpret_cast<int>(target);
				}

			}
			else if (symbol->shndx == SectionIndex::ABS)
			{
				// Absolute symbol
				return symbol->value;
			}
			else
			{
				// Internally defined symbol
				SectionHeader* target = getSection(header, static_cast<size_t>(symbol->shndx));
				return reinterpret_cast<int>(header) + symbol->value + target->offset;
			}
		}

	};

	void info(void* elfPtr)
	{
		INT_STR_BUFFER_ARR;
		Header* h = reinterpret_cast<Header*>(elfPtr);
		Vesa::writeLine("elf info for file at:", utoa(reinterpret_cast<uint32_t>(elfPtr), buf, 16));
		char typeBuf[4];
		typeBuf[3] = 0;
		memcpy(typeBuf, &h->ident[1], 3);
		Vesa::writeLine("type: ", utoa(static_cast<uint32_t>(h->type), buf, 10));
		Vesa::writeLine("machine: ", utoa(static_cast<uint32_t>(h->machine), buf, 10));
		Vesa::writeLine("version: ", utoa(h->version, buf, 10));
		Vesa::writeLine("entry: ", utoa(h->entry, buf, 16));
		Vesa::writeLine("section header offset: ", utoa(h->shoff, buf, 16));
		Vesa::writeLine("section header entries: ", utoa(h->shnum, buf, 16));
		Vesa::writeLine("section header entry size: ", utoa(h->shentsize, buf, 16));
	}

	bool isValid(void* elfPtr)
	{
		Header* h = reinterpret_cast<Header*>(elfPtr);
		bool matchClass = matchEnum(ClassType::CLASS_32, h->ident[II_CLASS]);
		bool matchData = matchEnum(DataType::DATA2LSB, h->ident[II_DATA]);
		bool matchMachine = Machine::M_386 == h->machine;
		return (matchClass && matchData && matchMachine);
	}

	bool load(void* buffer, size_t bufferSize)
	{
		if (!isValid(buffer))
		{
			ERROR("Invalid elf type!");
			return false;
		}
		else
		{
			return true;
		}
	}
};
