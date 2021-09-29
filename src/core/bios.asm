[bits 32]

section .bios

global call_bios_routine

call_bios_routine:
	cli
	pusha
	
	mov dword [sp_ptr], esp
	mov dword [bp_ptr], ebp
	
	mov ebp, esp
	mov eax, [ebp + 40] ; registers address
	mov ebp, eax
	
	mov ax, word [ebp + 0]
	mov bx, word [ebp + 2]
	mov cx, word [ebp + 4]
	mov dx, word [ebp + 6]
	mov di, word [ebp + 8]
	mov si, word [ebp + 10]

	mov word [bios_routine_registers + 0], ax
	mov word [bios_routine_registers + 2], bx
	mov word [bios_routine_registers + 4], cx
	mov word [bios_routine_registers + 6], dx
	mov word [bios_routine_registers + 8], di
	mov word [bios_routine_registers + 10], si

	jmp word 0x18:pm_16

[bits 16]
pm_16:
	mov ax, 0x20
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov eax, cr0
    and al, ~0x01
    mov cr0, eax

	jmp word 0x0:rm_16

rm_16:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	cld

	mov ebp, [esp + 36]
	
	mov eax, ebp

	mov bp, bios_stack + 0x200
	mov sp, bp

	mov ebp, eax

	mov ax, word [bios_routine_registers + 0]
	mov bx, word [bios_routine_registers + 2]
	mov cx, word [bios_routine_registers + 4]
	mov dx, word [bios_routine_registers + 6]
	mov di, word [bios_routine_registers + 8]
	mov si, word [bios_routine_registers + 10]
	
	sti

	call ebp
	
	cli

	mov eax, cr0
	or al, 1
	mov cr0, eax

	jmp 0x8:bios_routine_done

align 4

[bits 32]
bios_routine_done:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	cld

	mov ebp, dword [sp_ptr]
	mov esp, ebp
	mov ebp, dword [bp_ptr]

	popa

	ret

align 4

sp_ptr:				dd 0x0
bp_ptr:				dd 0x0

bios_routine_registers:
	.ax:			dw 0x0
	.bx:			dw 0x0
	.cx:			dw 0x0
	.dx:			dw 0x0
	.di:			dw 0x0
	.si:			dw 0x0
bios_routine_registers_end:

bios_stack:
	times 0x200 db 0
