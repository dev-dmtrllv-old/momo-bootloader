#pragma once

#include "core/types.hpp"

namespace PIC
{
	namespace Ports
	{
		constexpr uint8_t masterCommand = 0x20;
		constexpr uint8_t masterData = 0x21;
		constexpr uint8_t slaveCommand = 0xA0;
		constexpr uint8_t slaveData = 0xA1;
	}

	/**
	R	SL  EOI 	Description
	0	0	0		Rotate in Automatic EOI mode (CLEAR)
	0	0	1		Non specific EOI command
	0	1	0		No operation
	0	1	1		Specific EOI command
	1	0	0		Rotate in Automatic EOI mode (SET)
	1	0	1		Rotate on non specific EOI
	1	1	0		Set priority command
	1	1	1		Rotate on specific EOI 
	 */

	constexpr uint8_t eoi = 0x20; // end of interrupt (bit 5 of ICW2 = non specific eoi command)

	inline constexpr unsigned char operator "" _uchar(unsigned long long arg) noexcept
	{
		return static_cast<unsigned char>(arg);
	}

	enum class ICW1 : uint8_t
	{
		ICW4 = 0x01,			/* ICW4 (not) needed */
		SINGLE = 0x02,			/* Single (cascade) mode */
		INTERVAL4 = 0x04,		/* Call address interval 4 (8) */
		LEVEL = 0x08,			/* Level triggered (edge) mode */
		INIT = 0x10,			/* Initialization - required! */
	};

	enum class ICW2 : uint8_t
	{

	};

	constexpr uint8_t ICW4_8086 = 0x01;			/* 8086/88 (MCS-80/85) mode */
	constexpr uint8_t ICW4_AUTO = 0x02;			/* Auto (normal) EOI */
	constexpr uint8_t ICW4_BUF_SLAVE = 0x08;	/* Buffered mode/slave */
	constexpr uint8_t ICW4_BUF_MASTER = 0x0C;	/* Buffered mode/master */
	constexpr uint8_t ICW4_SFNM = 0x10;			/* Special fully nested (not) */

	inline void sendEOI(uint8_t irq);

	inline uint8_t readIMR();
	inline void writeIMR(uint8_t bits);

	void setIRQMask(uint8_t line, bool masked = true);

	void init();
};
