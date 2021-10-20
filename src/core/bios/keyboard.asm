[bits 16]

%include "src/core/bios/macro.asm"

BIOS_ROUTINE bios_get_keyboard_char
	xor ax, ax
	int 0x16
	ret
