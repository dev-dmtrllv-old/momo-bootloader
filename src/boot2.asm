%include "lib/defines.asm"

[org BOOT2_ADDR]
[bits 16]

boot_start:
	mov si, load_msg
	call print_line
	
	jmp halt

%include "lib/common.asm"
%include "lib/screen.asm"
%include "lib/disk.asm"

load_msg: db "Loading bootloader core...", 0

times 510 - ($-$$) db 0

dw 0xaa55
