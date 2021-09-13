%include "lib/defines.asm"

%macro error_handler 2
%1:
	mov si, %2
	call print_line
	jmp wait_shutdown
%endmacro

[org BOOT2_ADDR]
[bits 16]

boot_start:
	call check_a20
	cmp eax, 0
	jne a20_enabled
	call enable_a20
	cmp eax, 0
	jne a20_failed

a20_enabled:
	mov si, load_msg
	call print_line
	jmp halt

	jmp wait_shutdown




error_handler a20_failed, a20_fail_msg

%include "lib/common.asm"
%include "lib/screen.asm"
%include "lib/disk.asm"
%include "lib/a20.asm"

load_msg: 			db "Loading bootloader core...", 0
a20_fail_msg:		db "Failed to enable the A20 line!", 0

times 510 - ($-$$) db 0

dw 0xaa55
