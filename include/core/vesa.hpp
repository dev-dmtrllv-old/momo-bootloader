#pragma once

#include "core/types.hpp"

namespace Vesa
{
	struct Graphics
	{
		uint16_t mode;
		uint32_t width;
		uint32_t height;
		uint32_t pitch;
		uint8_t bpp;
		uint32_t frameBuffer;
		uint8_t rgbBitOffset[3];
	};

	bool init();
	uint16_t findClosestMode(uint32_t x, uint32_t y, uint32_t depth, bool useLinearFrameBuffer);
	bool setMode(uint16_t mode);

	void setPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);
	void fillRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint16_t r, uint16_t g, uint16_t b);
};
