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

		enum Types
		{
			LOW,
			HIGH,
			SIZE
		};

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

			call_bios_routine(&bios_get_mem_map, &regs);

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
					const uint32_t t = align(biosMemBuf[i].base + biosMemBuf[i].size);
					for (b; b < t; b += pageSize)
						if(b >= minBitMapAddr)
							setBitmapAtAddr(b, false);
				}
			}

			for(size_t base = alignDown(CoreInfo::getBinaryStartAddr()), top = align(CoreInfo::getBinaryEndAddr()); base < top; base += 0x1000)
				setBitmapAtAddr(base, false);

			isInitialized_ = true;
		}
		else
		{
			WARN("MM is already initialized...");
		}
	}

	void* getPage()
	{
		size_t index = 0;
		while(bitmapPtr_[index] != 0xFFFFFFFF)
		{
			for(uint8_t i = 0; i < bitmapIntSize; i++)
			{
				if(isBitmapFree(&bitmapPtr_[index], i))
				{
					setBitmap(&bitmapPtr_[index], i, false);
					return reinterpret_cast<void*>(bitmapToAddr(index, i));
				}
			}
			index++;
		}
	}

	void freePage(void* address)
	{
		setBitmapAtAddr(reinterpret_cast<uint32_t>(address), true);
	}
};
