#include "core/mm.hpp"
#include "core/bios.hpp"
#include "core/core_info.hpp"
#include "core/macros.hpp"
#include "core/vesa.hpp"
#include "core/string.hpp"

namespace MM
{
	namespace
	{
		struct BiosMemoryEntry
		{
			uint64_t base;
			uint64_t size;
			uint32_t usedFlag;
			uint32_t acpiFlag;
		} PACKED;

		struct RealModeMemoryEntry
		{
			uint16_t base;
			uint16_t size;
			bool isFree;
		} PACKED;



		constexpr size_t biosAddr = 0x2000;
		constexpr size_t biosSize = 0xE000;
		constexpr size_t biosTopAddr = biosAddr + biosSize;
		constexpr size_t maxAddr = 0x10000000;
		constexpr size_t bitmapIntSize = (sizeof(uint32_t) * 8);

		constexpr size_t getBitmapCount()
		{
			constexpr size_t a = (maxAddr - biosSize);
			if (a % bitmapIntSize == 0)
				return a / bitmapIntSize;
			return a / bitmapIntSize + 1;
		}

		constexpr size_t highBitmapCount = getBitmapCount();
		constexpr size_t highBitmapSize = align(highBitmapCount * sizeof(int));
		constexpr size_t highBitmapPagesCount = highBitmapSize / pageSize;



		bool isInitialized_ = false;
		uint32_t* bitmapPtr_ = nullptr;
		RealModeMemoryEntry* realModeMemoryList_ = nullptr;
		size_t maxRealModeEntries_ = 0;

		RealModeMemoryEntry* findFreeEntry()
		{
			size_t index = 0;
			while (index < maxRealModeEntries_)
			{
				if (realModeMemoryList_[index].size == 0)
					return &realModeMemoryList_[index];
				index++;
			}
			ERROR("Could not get a free real mode memory entry!");
			return nullptr;
		}

		RealModeMemoryEntry* addRealModeEntry(uint16_t base, uint16_t size, bool isFree)
		{
			RealModeMemoryEntry* e = findFreeEntry();
			if (e != nullptr)
			{
				e->base = base;
				e->size = size;
				e->isFree = isFree;
			}
			else
			{
				ERROR("add real mode entry fail...");
			}
			return e;
		}

		RealModeMemoryEntry* findRealModeEntry(uint32_t address)
		{
			size_t index = 0;
			while (index < maxRealModeEntries_)
			{
				if (realModeMemoryList_[index].base == address)
					return &realModeMemoryList_[index];
				index++;
			}
			return nullptr;
		}

		void setBitmap(uint32_t* bitmap, size_t offset, bool free)
		{
			if (free)
				*bitmap &= ~(1UL << offset);
			else
				*bitmap |= 1UL << offset;
		}

		bool isBitmapFree(uint32_t* bitmap, size_t offset)
		{
			return ((*bitmap >> offset) & 1U) == 0;
		}

		void setBitmapAtAddr(uint32_t addr, bool free)
		{
			const uint32_t a = (align(addr) - biosTopAddr);
			const uint32_t s = (pageSize * bitmapIntSize);
			const uint32_t index = a / s;
			const uint32_t offset = (a % s) / pageSize;

			if (free)
				bitmapPtr_[index] &= ~(1UL << offset);
			else
				bitmapPtr_[index] |= 1UL << offset;
		}

		bool isAddrFree(uint32_t addr)
		{
			const uint32_t dif = (align(addr) - biosTopAddr);
			const uint32_t s = (pageSize * bitmapIntSize);
			const uint32_t index = dif / s;
			const uint32_t offset = dif % s;
			return ((bitmapPtr_[index] >> offset) & 1U) == 0;
		}

		uint32_t bitmapToAddr(size_t index, size_t offset)
		{
			return biosTopAddr + (index * pageSize * bitmapIntSize) + (offset * pageSize);
		}
	};

	void init()
	{
		if (!isInitialized_)
		{
			BiosMemoryEntry* biosMemBuf;


			auto bufAddr = align(CoreInfo::getBiosEndAddr(), 0x10);
			biosMemBuf = reinterpret_cast<BiosMemoryEntry*>(bufAddr);

			Bios::Registers regs = {};
			regs.di = bufAddr;

			Bios::call(&bios_get_mem_map, &regs);

			const uint32_t minBitMapAddr = align(CoreInfo::getBinaryEndAddr());

			// find a free place to store the bit maps
			for (size_t i = 0; i < regs.cx; i++)
			{
				if (biosMemBuf[i].usedFlag == 1)
				{
					const uint32_t b = align(biosMemBuf[i].base);
					const uint32_t s = biosMemBuf[i].size;
					const uint64_t t = alignDown(b + (s - 1));

					if ((t >= minBitMapAddr && (t - minBitMapAddr) >= highBitmapSize))
					{
						bitmapPtr_ = reinterpret_cast<uint32_t*>(t - highBitmapSize);
						break;
					}
				}
			}

			if (bitmapPtr_ == nullptr)
			{
				ERROR("MM: Could not find enough space to allocate the bitmaps!");
				return;
			}

			for (size_t i = 0; i < regs.cx; i++)
			{
				if (biosMemBuf[i].usedFlag != 1)
				{
					uint32_t b = alignDown(biosMemBuf[i].base);
					if (b >= minBitMapAddr)
					{
						const uint32_t t = align(biosMemBuf[i].base + biosMemBuf[i].size);
						for (; b < t; b += pageSize)
							setBitmapAtAddr(b, false);
					}
				}
			}

			for (size_t base = alignDown(CoreInfo::getBinaryStartAddr()), top = align(CoreInfo::getBinaryEndAddr()); base < top; base += 0x1000)
				setBitmapAtAddr(base, false);


			realModeMemoryList_ = reinterpret_cast<RealModeMemoryEntry*>(getPage());
			maxRealModeEntries_ = pageSize / sizeof(RealModeMemoryEntry);

			// bufAddr was used for the bios memory map, and points to the end of the real mode code
			addRealModeEntry(0x0, bufAddr, false);
			addRealModeEntry(bufAddr, biosTopAddr - bufAddr, true);

			INT_STR_BUFFER_ARR;

			INFO(utoa(bufAddr, buf, 16));
			INFO(utoa(biosTopAddr - bufAddr, buf, 16));

			isInitialized_ = true;
		}
		else
		{
			WARN("MM is already initialized...");
		}
	}

	void* getPages(size_t bytes, size_t* numberOfPages)
	{
		const size_t totalNeedePages = MM::align(bytes) / MM::pageSize;
		*numberOfPages = totalNeedePages;
		
		size_t startIndex = 0;
		size_t startOffset = 0;
		size_t freePagesCount = 0;

		for (size_t index = 0; index < highBitmapCount; index++)
		{
			if (bitmapPtr_[index] != 0xFFFFFFFF)
			{
				for (uint8_t i = 0; i < bitmapIntSize; i++)
				{
					if (isBitmapFree(&bitmapPtr_[index], i))
					{
						if (freePagesCount == 0)
						{
							startIndex = index;
							startOffset = i;

						}

						freePagesCount++;

						if (freePagesCount == totalNeedePages)
						{
							void* buf = reinterpret_cast<void*>(bitmapToAddr(startIndex, startOffset));

							for (size_t j = 0; j < totalNeedePages; j++)
							{
								setBitmap(&bitmapPtr_[startIndex], startOffset++, false);
								if (startOffset >= bitmapIntSize)
								{
									startOffset = 0;
									startIndex++;
								}
							}
							memset(buf, 0, MM::pageSize * *numberOfPages);
							return buf;
						}

					}
					else
					{
						freePagesCount = 0;
					}
				}
			}
			else
			{
				freePagesCount = 0;
			}
		}
		return nullptr;

	}

	void* getPage()
	{
		for (size_t index = 0; index < highBitmapCount; index++)
		{
			if (bitmapPtr_[index] != 0xFFFFFFFF)
			{
				for (uint8_t i = 0; i < bitmapIntSize; i++)
				{
					if (isBitmapFree(&bitmapPtr_[index], i))
					{
						setBitmap(&bitmapPtr_[index], i, false);
						memset(reinterpret_cast<void*>(bitmapToAddr(index, i)), 0, MM::pageSize);
						return reinterpret_cast<void*>(bitmapToAddr(index, i));
					}
				}
			}
		}
		return nullptr;
	}

	void freePage(void* address)
	{
		setBitmapAtAddr(reinterpret_cast<uint32_t>(address), true);
	}

	void freePages(void* address, size_t pages)
	{
		for (size_t i = 0; i < pages; i++)
			setBitmapAtAddr(reinterpret_cast<uint32_t>(address) + (i * MM::pageSize), true);
	}

	void* allocRealModeBuffer(size_t size)
	{
		size_t index = 0;
		while (index < maxRealModeEntries_)
		{
			uint16_t b = realModeMemoryList_[index].base;
			uint16_t s = realModeMemoryList_[index].size;
			uint16_t t = b + (s - 1);

			if (s == 0 || !realModeMemoryList_[index].isFree || s < size)
			{
				index++;
				continue;
			}

			if (s == size)
			{
				realModeMemoryList_[index].isFree = false;
				memset(reinterpret_cast<void*>(b), 0, size);
				return reinterpret_cast<void*>(b);
			}
			else
			{
				// split the block
				realModeMemoryList_[index].size -= size;
				RealModeMemoryEntry* e = addRealModeEntry(b + realModeMemoryList_[index].size, size, false);
				memset(reinterpret_cast<void*>(b), 0, size);
				return reinterpret_cast<void*>(e->base);
			}

			index++;
		}
		return nullptr;
	}

	void freeRealModeBuffer(void* buffer)
	{
		RealModeMemoryEntry* e = findRealModeEntry(reinterpret_cast<uint32_t>(buffer));

		if (e != nullptr)
		{
			size_t index = 0;

			// find a free block where we can merge on top off
			while (index < maxRealModeEntries_)
			{
				if (realModeMemoryList_[index].size == 0 || !realModeMemoryList_[index].isFree)
				{
					index++;
					continue;
				}
				else if (realModeMemoryList_[index].base + realModeMemoryList_[index].size == e->base)
				{
					// merge with block at index
					realModeMemoryList_[index].size += e->size;
					e->size = 0;
					e->base = 0;
					e->isFree = false;
					return;
				}
				index++;
			}

			index = 0;
			while (index < maxRealModeEntries_)
			{
				if (realModeMemoryList_[index].size == 0 || !realModeMemoryList_[index].isFree)
				{
					index++;
					continue;
				}
				else if (realModeMemoryList_[index].base == e->base + e->size)
				{
					// merge with block at index
					realModeMemoryList_[index].size += e->size;
					realModeMemoryList_[index].base -= e->size;
					e->size = 0;
					e->base = 0;
					e->isFree = false;
					return;
				}
				index++;
			}

			// else simply set the isFree flag to true
			e->isFree = true;
		}
		else
		{
			WARN("Could not free real mode buffer!");
		}

	}
};
