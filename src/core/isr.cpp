#include "core/isr.hpp"

namespace ISR
{
	namespace
	{
		__attribute__((aligned(0x10))) static Entry idtEntries[256];

		static IDTR idtr;

		extern "C"
		{
			void isr0(); // Division by zero exception
			void isr1(); // Debug exception
			void isr2(); // Non maskable interrupt
			void isr3(); // Breakpoint exception
			void isr4(); // 'Into detected overflow'
			void isr5(); // Out of bounds exception
			void isr6(); // Invalid opcode exception
			void isr7(); // No coprocessor exception
			void isr8(); // Double fault (pushes an error code)
			void isr9(); // Coprocessor segment overrun
			void isr10(); // Bad TSS (pushes an error code)
			void isr11(); // Segment not present (pushes an error code)
			void isr12(); // Stack fault (pushes an error code)
			void isr13(); // General protection fault (pushes an error code)
			void isr14(); // Page fault (pushes an error code)
			void isr15(); // Unknown interrupt exception
			void isr16(); // Coprocessor fault
			void isr17(); // Alignment check exception
			void isr18(); // Machine check exception
			void isr19(); // reserved
			void isr20(); // reserved
			void isr21(); // reserved
			void isr22(); // reserved
			void isr23(); // reserved
			void isr24(); // reserved
			void isr25(); // reserved
			void isr26(); // reserved
			void isr27(); // reserved
			void isr28(); // reserved
			void isr29(); // reserved
			void isr30(); // reserved
			void isr31(); // reserved

			void idt_flush(uint32_t ptr);

			void isr_handler(Registers* registers)
			{
				Vesa::writeLine("got interrupt");
				// if (isr::interrupt_handlers[registers->int_no] != 0)
				// {
				// 	isr::interrupt_handler_callback handler = isr::interrupt_handlers[registers->int_no];
				// 	handler(registers);
				// }
				// else
				// {
				// 	char buf[16];
				// 	utoa(registers->int_no, buf, 10);
				// 	vga::write_line("isr ", buf, " recieved!");

				// 	if (registers->int_no == 13)
				// 	{
				// 		utoa(registers->err_code, buf, 10);
				// 		vga::write_line("general protection fault. err: ", buf);
				__asm__ volatile("cli; hlt;");
				// 	}
				// }
			}
		};

		void setGate(uint32_t index, uint32_t isr, uint8_t cs, uint8_t attr)
		{
			idtEntries[index].highAddr = (isr >> 16) & 0xFFFF;
			idtEntries[index].lowAddr = static_cast<uint16_t>(isr);

			idtEntries[index].cs = cs;
			idtEntries[index].reserved = 0;
			// We must uncomment the OR below when we get to using user-mode.
			// It sets the interrupt gate's privilege level to 3.
			idtEntries[index].attributes = attr /* | 0x60 */;
		}
	};

	void init()
	{
		idtr.base = reinterpret_cast<uint32_t>(&idtEntries);
		idtr.limit = (sizeof(Entry) * 256) - 1;

		setGate(0, (uint32_t)isr0, 0x08, 0x8E);
		setGate(1, (uint32_t)isr1, 0x08, 0x8E);
		setGate(2, (uint32_t)isr2, 0x08, 0x8E);
		setGate(3, (uint32_t)isr3, 0x08, 0x8E);
		setGate(4, (uint32_t)isr4, 0x08, 0x8E);
		setGate(5, (uint32_t)isr5, 0x08, 0x8E);
		setGate(6, (uint32_t)isr6, 0x08, 0x8E);
		setGate(7, (uint32_t)isr7, 0x08, 0x8E);
		setGate(8, (uint32_t)isr8, 0x08, 0x8E);
		setGate(9, (uint32_t)isr9, 0x08, 0x8E);
		setGate(10, (uint32_t)isr10, 0x08, 0x8E);
		setGate(11, (uint32_t)isr11, 0x08, 0x8E);
		setGate(12, (uint32_t)isr12, 0x08, 0x8E);
		setGate(13, (uint32_t)isr13, 0x08, 0x8E);
		setGate(14, (uint32_t)isr14, 0x08, 0x8E);
		setGate(15, (uint32_t)isr15, 0x08, 0x8E);
		setGate(16, (uint32_t)isr16, 0x08, 0x8E);
		setGate(17, (uint32_t)isr17, 0x08, 0x8E);
		setGate(18, (uint32_t)isr18, 0x08, 0x8E);
		setGate(19, (uint32_t)isr19, 0x08, 0x8E);
		setGate(20, (uint32_t)isr20, 0x08, 0x8E);
		setGate(21, (uint32_t)isr21, 0x08, 0x8E);
		setGate(22, (uint32_t)isr22, 0x08, 0x8E);
		setGate(23, (uint32_t)isr23, 0x08, 0x8E);
		setGate(24, (uint32_t)isr24, 0x08, 0x8E);
		setGate(25, (uint32_t)isr25, 0x08, 0x8E);
		setGate(26, (uint32_t)isr26, 0x08, 0x8E);
		setGate(27, (uint32_t)isr27, 0x08, 0x8E);
		setGate(28, (uint32_t)isr28, 0x08, 0x8E);
		setGate(29, (uint32_t)isr29, 0x08, 0x8E);
		setGate(30, (uint32_t)isr30, 0x08, 0x8E);
		setGate(31, (uint32_t)isr31, 0x08, 0x8E);

		idt_flush(reinterpret_cast<uint32_t>(&idtr));
	};
};
