#include "core/vesa.hpp"
#include "core/types.hpp"
#include "core/mm.hpp"
#include "core/vga.hpp"
#include "core/string.hpp"
#include "core/bios.hpp"

namespace Vesa
{
	namespace
	{
		struct VbeInfoBlock
		{
			char vbeSignature[4]; // == "VESA"
			uint16_t vbeVersion;  // == 0x0300 for VBE 3.0
			uint32_t oemStringPtr;
			uint8_t capabilities[4];
			uint32_t videoModePtr;		// pointer to mode list
			uint16_t totalMemory;		// as # of 64KB blocks
			uint16_t oemSoftwareRev;	// VBE implementation Software revision
			uint32_t oemVendorNamePtr;	// Pointer to Vendor Name String
			uint32_t oemProductNamePtr; // Pointer to Product Name String
			uint32_t oemProductRevPtr;	// Pointer to Product Revision String
			uint8_t reserved[222];		// Reserved for VBE implementation scratch area
			uint8_t oemData[256];		// Data Area for OEM Strings
		} __attribute__((packed));

		struct VbeModeInfo
		{
			uint16_t attributes;  // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
			uint8_t windowA;	  // deprecated
			uint8_t windowB;	  // deprecated
			uint16_t granularity; // deprecated; used while calculating bank numbers
			uint16_t windowSize;
			uint16_t segmentA;
			uint16_t segmentB;
			uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode without returning to real mode
			uint16_t pitch;		   // number of bytes per horizontal line
			uint16_t width;		   // width in pixels
			uint16_t height;	   // height in pixels
			uint8_t wChar;		   // unused...
			uint8_t yChar;		   // ...
			uint8_t planes;
			uint8_t bpp;   // bits per pixel in this mode
			uint8_t banks; // deprecated; total number of banks in this mode
			uint8_t memoryModel;
			uint8_t bankSize; // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
			uint8_t image_pages;
			uint8_t reserved0;

			uint8_t redMask;
			uint8_t redPosition;
			uint8_t greenMask;
			uint8_t greenPosition;
			uint8_t blueMask;
			uint8_t bluePosition;
			uint8_t reservedMask;
			uint8_t reservedPosition;
			uint8_t directColorAttributes;

			uint32_t framebuffer; // physical address of the linear frame buffer; write here to draw to the screen
			uint32_t offScreenMemoff;
			uint16_t offScreenMemSize; // size of memory in the framebuffer but not being displayed on the screen
			uint8_t reserved1[206];
		} __attribute__((packed));

		bool isInitialized_ = false;

		VbeInfoBlock *vib_ = nullptr;

		Graphics gfx;

		inline bool interruptSuccess(const Bios::Registers &regs) { return regs.eax == 0x4F; }

		bool getControllerInfo(VbeInfoBlock *infoBlock)
		{
			Bios::Registers regs = {.eax = 0x4f00, .edi = reinterpret_cast<uint32_t>(infoBlock)};

			BIOS_INTERRUPT($0x10, regs);

			return interruptSuccess(regs);
		}

		void validateControllerInfo(VbeInfoBlock *info)
		{
			char buf[16];
			Vga::print(utoa(reinterpret_cast<uint32_t>(info), buf, 16));
			memcpy(buf, info->vbeSignature, 4);
			buf[4] = '\0';
			Vga::print(buf);
			Vga::print("TODO validate controller info!!!");
		}

		bool getModeInfo(uint32_t mode, VbeModeInfo *modeInfo)
		{
			memset(reinterpret_cast<void *>(modeInfo), 0, sizeof(VbeModeInfo));

			Bios::Registers regs = {.eax = 0x4f01, .ecx = mode, .edi = reinterpret_cast<uint32_t>(modeInfo)};

			BIOS_INTERRUPT($0x10, regs);

			return interruptSuccess(regs);
		}

		inline uint32_t diff(uint32_t a, uint32_t b) { return a > b ? a - b : b - a; };

		inline uint8_t *coordsToAddr(uint16_t x, uint16_t y) { return reinterpret_cast<uint8_t *>((y * gfx.pitch) + (x * (gfx.bpp / 8)) + reinterpret_cast<uint32_t>(gfx.frameBuffer)); }

	};

	bool init()
	{
		if (!isInitialized_)
		{
			memset(&gfx, 0, sizeof(Graphics));

			vib_ = MM::alloc<VbeInfoBlock>(512);
			memcpy(vib_->vbeSignature, "VBE2", 4);

			if (!getControllerInfo(vib_))
				return false;

			validateControllerInfo(vib_);

			isInitialized_ = true;
			return true;
		}
		return false;
	}

	uint16_t findClosestMode(uint32_t x, uint32_t y, uint32_t depth, bool useLinearFrameBuffer)
	{
		uint16_t best = 0x13;

		char buf[16];

		int pixdiff, bestpixdiff = diff(320 * 200, x * y);
		int depthdiff, bestdepthdiff = 8 >= depth ? 8 - depth : (depth - 8) * 2;

		uint16_t *modes = reinterpret_cast<uint16_t *>(vib_->videoModePtr);

		VbeModeInfo *modeInfo = MM::alloc<VbeModeInfo>(sizeof(VbeModeInfo));

		for (size_t i = 0; modes[i] != 0xFFFF; ++i)
		{
			uint16_t modeNum = (modes[i] >> 8) + ((modes[i] & 0xFF) << 8);

			Vga::print("mode ");
			Vga::print(utoa(modes[i], buf, 16));
			Vga::print(": ");

			if (!getModeInfo(modes[i], modeInfo))
			{
				Vga::print("skip\n");
				continue;
			}

			Vga::print(utoa(modeInfo->width, buf, 10));
			Vga::print("x");
			Vga::print(utoa(modeInfo->height, buf, 10));
			Vga::print("\n");

			// Check if this is a graphics mode with linear frame buffer support
			if ((modeInfo->attributes & 0x90) != 0x90)
				continue;

			// Check if this is a packed pixel or direct color mode
			if (modeInfo->memoryModel != 4 && modeInfo->memoryModel != 6)
				continue;

			// Check if this is exactly the mode we're looking for
			if (x == modeInfo->width && y == modeInfo->height && depth == modeInfo->bpp)
			{
				// *frameBuffAddr = modeInfo->framebuffer;
				MM::free(modeInfo);
				return modes[i];
			}

			pixdiff = diff(modeInfo->width * modeInfo->height, x * y);
			depthdiff = (modeInfo->bpp >= depth) ? modeInfo->bpp - depth : (depth - modeInfo->bpp) * 2;
			if (bestpixdiff > pixdiff || bestpixdiff == pixdiff)
			{
				best = modes[i];
				bestpixdiff = pixdiff;
				bestdepthdiff = depthdiff;
			}
		}
		MM::free(modeInfo);
		return best;
	}

	bool setMode(uint16_t mode)
	{
		Bios::Registers regs = {.eax = 0x4f02, .ebx = mode};

		BIOS_INTERRUPT($0x10, regs);

		if (!interruptSuccess(regs))
		{
			return false;
		}
		else
		{
			VbeModeInfo *modeInfo = MM::alloc<VbeModeInfo>(sizeof(VbeModeInfo));

			if (!getModeInfo(mode, modeInfo))
			{
				MM::free(modeInfo);
				return false;
			}

			gfx.mode = mode;
			gfx.bpp = modeInfo->bpp;
			gfx.frameBuffer = modeInfo->framebuffer;
			gfx.height = modeInfo->height;
			gfx.width = modeInfo->width;
			gfx.rgbBitOffset[0] = modeInfo->redPosition;
			gfx.rgbBitOffset[1] = modeInfo->greenPosition;
			gfx.rgbBitOffset[2] = modeInfo->bluePosition;
			gfx.pitch = modeInfo->pitch;
			
			MM::free(modeInfo);

			return true;
		}
	}

	void setPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		uint8_t *pixel = coordsToAddr(x, y);

		*pixel++ = b;
		*pixel++ = g;
		*pixel = r;
	}

	void fillRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint16_t r, uint16_t g, uint16_t b, uint16_t a)
	{
		const size_t pixelWidth = gfx.bpp / 8;
		uint8_t *where = coordsToAddr(x, y);

		for (size_t i = 0; i < height; i++)
		{
			uint8_t *p = where;
			for (size_t j = 0; j < width; j++)
			{
				*p++ = b;
				*p++ = g;
				*p = r;
				p += 2;
			}
			where += gfx.pitch;
		}
	}
};
