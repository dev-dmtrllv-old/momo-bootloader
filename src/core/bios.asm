[bits 32]

section .bios

global call_bios_routine

call_bios_routine:
	cli
	pusha

	mov dword [sp_ptr], esp
	mov dword [bp_ptr], ebp

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

	sti

	mov eax, [esp + 36]
	mov ebx, [esp + 40]

	call eax
	
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


sp_ptr:		dd 0x0
bp_ptr:		dd 0x0
