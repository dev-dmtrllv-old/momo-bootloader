#include "core/mm.hpp"

#include "core/string.hpp"
#include "core/vga.hpp"
#include "core/keyboard.hpp"
#include "core/core_info.hpp"

namespace MM
{
	struct BiosMemBlock
	{
		uint64_t base;
		uint64_t size;
		uint32_t freeFlag;
		uint32_t acpiFlags;
	} __attribute__((packed));

	struct MemBlock
	{
		uint64_t base;
		uint64_t size;
		bool isUsed;
	} __attribute__((packed));

	struct FixedReservedBlock
	{
		uint64_t base;
		uint64_t top;
	} __attribute__((packed));

	namespace
	{
		bool isInitialized_ = false;
	};

	namespace Pool
	{
		constexpr uint32_t maxListSize = 64;

		struct MemBlockList
		{
			size_t freeBlockCount;
			MemBlock blocks[maxListSize];
			MemBlockList *next;
		} __attribute__((packed));

		MemBlockList memList;
		MemBlockList *lastList;

		MemBlock *findBlockToUse(MemBlockList *list)
		{
			for (size_t i = 0; i < Pool::maxListSize; i++)
			{
				MemBlock *b = &list->blocks[i];
				if (b->base == 0 && b->size == 0)
					return b;
			}
			return nullptr;
		}

		MemBlock *findBlockWithSize(MemBlockList *list, uint64_t size)
		{
			for (size_t i = 0; i < Pool::maxListSize; i++)
			{
				MemBlock *b = &list->blocks[i];
				if (b->isUsed == false && b->size >= size)
					return b;
			}
			return nullptr;
		}

		MemBlockList *allocList()
		{
			MemBlockList *l = &memList;
			while (l != nullptr)
			{
				MemBlock *b = findBlockWithSize(l, sizeof(MemBlockList));
				if (b != nullptr)
				{
					MemBlockList *newList = reinterpret_cast<MemBlockList *>(b->base);
					memset(newList, 0, sizeof(MemBlockList));

					if (b->size == sizeof(MemBlockList))
					{
						b->isUsed = true;
						newList->freeBlockCount = Pool::maxListSize;
					}
					else
					{
						b->size -= sizeof(MemBlockList);
						newList->freeBlockCount = Pool::maxListSize - 1;
						newList->blocks[0].base = b->base + b->size;
						newList->blocks[0].size = sizeof(MemBlockList);
						newList->blocks[0].isUsed = true;
					}
					return newList;
				}
				l = l->next;
			}
			// PANIC !!!!!
			// not enough memory !!!

			return nullptr;
		}

		MemBlockList *getFreeList()
		{
			MemBlockList *l = &memList;
			while (l != nullptr)
			{
				if (l->freeBlockCount > 0)
					return l;
				l = l->next;
			}
			// no free list found, lets allocate a new one
			lastList->next = allocList();
			lastList = lastList->next;
			return lastList;
		}

		void setBlock(uint64_t base, uint64_t size, bool isUsed)
		{
			MemBlockList *l = getFreeList();
			MemBlock *b = findBlockToUse(l);
			b->base = base;
			b->size = size;
			b->isUsed = isUsed;
			l->freeBlockCount--;
		}

		void init()
		{
			memset(&memList, 0, sizeof(MemBlockList));
			memList.freeBlockCount = Pool::maxListSize;
			lastList = &memList;
		}

		MemBlock *getBlock(uint64_t addr, Pool::MemBlockList **listPtr)
		{
			Pool::MemBlockList *l = &Pool::memList;
			while (l != nullptr)
			{
				for (size_t i = 0; i < Pool::maxListSize; i++)
					if (addr >= l->blocks[i].base && addr <= l->blocks[i].base + l->blocks[i].size - 1)
					{
						if (listPtr != nullptr)
							*listPtr = l;
						return &l->blocks[i];
					}
				l = l->next;
			}
			return nullptr;
		}
	};

	inline uint32_t align(uint32_t addr, uint32_t alignment)
	{
		if (addr % alignment == 0)
			return addr;
		return ((addr / alignment) + 1) * alignment;
	}

	void *alloc(size_t size)
	{
		Pool::MemBlockList *l = &Pool::memList;
		while (l != nullptr)
		{
			if (l->freeBlockCount > 0)
			{
				for (size_t i = 0; i < Pool::maxListSize; i++)
				{
					MemBlock *b = &l->blocks[i];
					if (!b->isUsed)
					{
						if (b->size > size)
						{
							b->size -= size;
							uint32_t allocAddr = b->base + b->size;
							Pool::setBlock(allocAddr, size, true);
							return reinterpret_cast<void *>(allocAddr);
						}
						else if (b->size == size)
						{
							b->isUsed = true;
							return reinterpret_cast<void *>(b->base);
						}
					}
				}
			}
			l = l->next;
		}
		return nullptr;
	}

	void free(void *addr)
	{
		Pool::MemBlockList *bl;
		Pool::MemBlockList *abovel;

		MemBlock *b = Pool::getBlock(reinterpret_cast<uint64_t>(addr), &bl);
		MemBlock *above = Pool::getBlock(reinterpret_cast<uint64_t>(addr) + b->size, &abovel);
		MemBlock *below = Pool::getBlock(reinterpret_cast<uint64_t>(addr) - 1, nullptr);

		const bool isAboveFree = (above != nullptr && !above->isUsed);
		const bool isBelowFree = (below != nullptr && !below->isUsed);

		b->isUsed = false;
		if (isAboveFree)
		{
			above->base -= b->size;
			above->size += b->size;
			b->base = 0;
			b->size = 0;
			b->isUsed = false;
			b = above;
			bl->freeBlockCount++;
			bl = abovel;
		}
		if (isBelowFree)
		{
			below->size += b->size;
			b->base = 0;
			b->size = 0;
			b->isUsed = false;
			bl->freeBlockCount++;
		}
	}

	void init(void *memMap, uint32_t size)
	{
		if (!isInitialized_)
		{
			Pool::init();

			BiosMemBlock *m = reinterpret_cast<BiosMemBlock *>(memMap);

			uint32_t blockCount = 0;

			// set the first block which is at address 0x0
			for (size_t i = 0; i < size; i++)
			{
				if (m[i].base == 0 && m[i].size == 0)
					continue;
				blockCount++;
				if (m[i].base == 0)
					Pool::setBlock(m[i].base, m[i].size, m[i].freeFlag != 0x1);
			}

			uint64_t lastBaseAddr = 0;
			size_t blocksSet = 1;

			for (size_t i = 0; i < blockCount - 1; i++)
			{
				uint64_t lowestFoundAddr = 0xFFFFFFFFFFFFFFFF;
				size_t index = 0;
				for (size_t j = 0; j < size; j++)
				{
					if (m[j].base == 0 && m[j].size == 0)
						continue;
					if (m[j].base > lastBaseAddr && m[j].base < lowestFoundAddr)
					{
						lowestFoundAddr = m[j].base;
						index = j;
					}
				}
				Pool::setBlock(m[index].base, m[index].size, m[index].freeFlag != 0x1);
				lastBaseAddr = m[index].base;
				blocksSet++;
				if (blocksSet == blockCount)
					break;
			}

			isInitialized_ = true;
		}
	}
}
