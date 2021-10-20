[bits 16]

%include "src/core/bios/macro.asm"

BIOS_ROUTINE bios_shutdown
	mov ah, 0x53
	mov al, 0x0
	xor bx, bx
	int 0x15
	jc apm_error

	mov ah, 0x53
	mov al, 0x04
	xor bx, bx
	int 0x15
	jc .disconnect_error
	jmp .no_error

	.disconnect_error:
		cmp ah, 0x03
		jne apm_error

	.no_error:

	mov ah, 0x53
	mov al, 0x01
	xor bx, bx
	int 0x15
	jc apm_error

	mov ah, 0x53
	mov al, 0x08
	mov bx, 0x01
	mov cx, 0x01
	int 0x15
	jc apm_error

	mov ah, 0x53
	mov al, 0x07
	mov bx, 0x1
	mov cx, 0x03
	int 0x15
	jc apm_error

	jmp hang

apm_error:
	mov si, apm_erro_msg
	print_char:
	lodsb
	or al, al
	jz .print_done
	mov ah, 0x0e
	int 0x10
	jmp print_char
	.print_done:
		ret

hang:
	cli
	hlt
	jmp $

apm_erro_msg:	db "APM error! Shutdown the computer manually...", 0
