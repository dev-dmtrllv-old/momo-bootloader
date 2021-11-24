#include "core/pic.hpp"
#include "core/io.hpp"

namespace PIC
{
	void sendEOI(uint8_t irq)
	{
		if (irq >= 8)
			IO::outb(Ports::slaveCommand, eoi);
		IO::outb(Ports::masterCommand, eoi);
	}

	uint8_t readIMR()
	{
		return IO::inb(Ports::masterData);
	}

	void writeIMR(uint8_t bits)
	{
		IO::outb(Ports::masterData, bits);
	}

	void setIRQMask(uint8_t line, bool masked)
	{
		uint8_t mask = readIMR();
		
		if(masked)
			mask |= 1 << line;
		else
			mask &= ~(1 << line);

		writeIMR(mask);
	}

	void init()
	{
		// send init command (ICW1)
		IO::outb(Ports::masterCommand, orEnum<uint8_t>(ICW1::INIT, ICW1::ICW4));
		IO::outb(Ports::slaveCommand, orEnum<uint8_t>(ICW1::INIT, ICW1::ICW4));

		// map IRQ 0...8...16 to 0x20...0x28...0x30 (ICW2)
		IO::outb(Ports::masterData, 0x20);
		IO::outb(Ports::slaveData, 0x28);

		// master <-> slave pic line (ICW3)
		IO::outb(Ports::masterData, 0b0100); // bitwise representation (2nd bit = line 2)
		IO::outb(Ports::slaveData, 0x2); // binary representation

		// set operation flags (ICW4)
		IO::outb(Ports::masterData, 0x1);
		IO::outb(Ports::slaveData, 0x1);

		for(uint8_t i = 0; i < 16; i++)
			setIRQMask(i, true);
	}
};
