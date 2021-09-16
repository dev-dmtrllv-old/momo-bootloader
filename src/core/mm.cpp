#include "core/mm.hpp"

#include "core/string.hpp"
#include "core/vga.hpp"
#include "core/keyboard.hpp"

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
		MemBlock *next;
	} __attribute__((packed));
	
	struct FixedReservedBlock
	{
		uint64_t base;
		uint64_t top;
	} __attribute__((packed));

	namespace
	{
		MemBlock *memBlockList_ = nullptr;
		MemBlock *lastMemBlock_ = nullptr;
		bool isInitialized_ = false;

		void createInitMemMap(BiosMemBlock *memMap, uint32_t size, uint64_t)
		{

			BiosMemBlock addedBlocks[] = {
				{ 0x0, 0x1000, }, // set the GDT and IVT as reserved
				{ 0x0, 0x1000, 0x2, 0x0 }
			};

			// for (size_t i = 0; i < size; i++)
			// {
			// 	copyAddr[i].base = memMap[i].base;
			// 	copyAddr[i].size = memMap[i].size;
			// 	copyAddr[i].isUsed = memMap[i].freeFlag != 1;
			// 	copyAddr[i].next = nullptr;
			// 	lastMemBlock_ = &copyAddr[i];
			// 	if (i != 0)
			// 		copyAddr[i - 1].next = &copyAddr[i];
			// }
		}

		MemBlock *getBlock(uint64_t addr)
		{
			MemBlock *b = memBlockList_;
			while (b != nullptr)
			{
				if (addr >= b->base && addr <= (b->base + b->size))
					return b;
				b = b->next;
			}
			return nullptr;
		}

		void setNewMemBlock(uint64_t base, uint64_t size, bool isUsed)
		{
			MemBlock *b = reinterpret_cast<MemBlock *>(base + size);
			b->base = base;
			b->size = size = sizeof(MemBlock);
			b->isUsed = isUsed;
			b->next = nullptr;
			lastMemBlock_->next = b;
			lastMemBlock_ = b;
		}

		void setUsed(uint64_t addr, uint64_t size)
		{
			
		}
	};

	namespace Pool
	{
		constexpr uint32_t maxListSize = 64;
		constexpr uint32_t listByteSize = sizeof(MemBlock) * maxListSize;
	};

	inline uint32_t align(uint32_t addr, uint32_t alignment)
	{
		if (addr % alignment == 0)
			return addr;
		return ((addr / alignment) + 1) * alignment;
	}

	void alloc(size_t size)
	{
	}

	void loop()
	{
		MemBlock *b = memBlockList_;
		while (b != nullptr)
		{
			char buf1[32];
			char buf2[32];

			utoa(static_cast<uint32_t>(b->base), buf1, 16);
			utoa(static_cast<uint32_t>(b->size), buf2, 16);

			VGA::print(buf1);
			VGA::print(" : ");
			VGA::print(buf2);
			if(!b->isUsed)
				VGA::print("    free");
			VGA::print("\n");

			b = b->next;
		}
		VGA::print("\n");
	}

	void init(void *memMap, uint32_t size, uint32_t coreBinarySize)
	{
		if (!isInitialized_)
		{
			BiosMemBlock *m = reinterpret_cast<BiosMemBlock *>(memMap);

			for(size_t i = 0; i < size; i++)
			{
				char buf1[16];
				char buf2[16];
				utoa(m[i].base, buf1, 16);
				utoa(m[i].size, buf2, 16);
				VGA::print(buf1);
				VGA::print(" : ");
				VGA::print(buf2);
				VGA::print(" (");
				VGA::print(m[i].freeFlag == 0x1 ? "free" : "used");
				VGA::print(")\n");
			}

			// const uint32_t biosMemMapTopAddr = align(reinterpret_cast<uint32_t>(memMap) + (sizeof(BiosMemBlock) * size), 0x10);

			// const size_t initMemMapSize = (sizeof(MemBlock) * size + 2);

			// createInitMemMap(m, size, reinterpret_cast<MemBlock *>(0x5000 - initMemMapSize));

			// loop();

			// setUsed(0x0, 0x1000);
			// setUsed(0x5000, coreBinarySize);

			// char buf[16];
			// VGA::print(utoa(reinterpret_cast<uint64_t>(memBlockList_), buf, 16));
			// VGA::print("\n");

			// setUsed(reinterpret_cast<uint64_t>(memBlockList_), initMemMapSize - (2 * sizeof(MemBlock)));

			// loop();

			isInitialized_ = true;
		}
	}
}
