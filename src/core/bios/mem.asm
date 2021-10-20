[bits 16]

%include "src/core/bios/macro.asm"

%define MEM_MAGIC 0x534D4150
%define GET_MEM_FUNC 0xE820

; di = start pointer to memory map buffer
; ax = 0xE820 (or ax = 0 on fail)
; cx = number of entries
BIOS_ROUTINE bios_get_mem_map
	mov bp, 0
	xor bx, bx	
	mov edx, MEM_MAGIC
	mov ax, GET_MEM_FUNC
	
	mov [es:di + 20], dword 1
	mov cx, 24

	int 0x15
	
	jc .get_mem_list_err
	cmp eax, MEM_MAGIC
	jne .get_mem_list_err

	test bx, bx
	je .get_mem_list_done
	
	mov [es:di + 20], dword 1
	add di, 24

	.get_mem_loop:
		inc bp
		mov edx, MEM_MAGIC
		mov ax, GET_MEM_FUNC
		mov cx, 24
		
		int 0x15
		
		jc .get_mem_list_done
		mov [es:di + 20], dword 1
		add di, 24
		test bx, bx
		jne .get_mem_loop

	.get_mem_list_done:
		mov cx, bp
		mov ax, GET_MEM_FUNC
		ret

	.get_mem_list_err:
		xor ax, ax
		ret
		