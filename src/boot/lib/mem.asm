
%define MEM_MAGIC 0x534D4150
%define GET_MEM_FUNC 0xE820

get_mem_list:
	mov bp, 0						; store the list count in bp
	xor ebx, ebx					; ebx should be zero as first entry
	mov edx, MEM_MAGIC				; set magic number
	mov eax, GET_MEM_FUNC			; set func
	mov ecx, 24						; set number of bytes to fetch
	int 0x15						; call bios 
	jc .get_mem_list_err			; carry on first call means error
	cmp eax, MEM_MAGIC				; eax will be set to the magic number
	jne .get_mem_list_err			; error if it's not set

	test ebx, ebx					; check if ebx is zero
	je .get_mem_list_done			; if so the list is 1 entry ???
	
	mov [es:di + 20], dword 1		; set ACPI 3.0 bits
	add di, 24						; move entry pointer to next location

	.get_mem_loop:
		inc bp						; add 1 to the entry counter
		mov edx, MEM_MAGIC			; set magic number		
		mov eax, GET_MEM_FUNC		; set func 
		mov ecx, 24					; set number of bytes to fetch
		int 0x15					; call bios
		jc .get_mem_list_done		; carry means end of list already reached
		mov [es:di + 20], dword 1	; set ACPI 3.0 bits
		add di, 24					; move entry pointer to next location
		test ebx, ebx				; check if ebx is zero
		jne .get_mem_loop			; if not get next entry

	.get_mem_list_done:
		clc							; clear the carry flag
		ret

	.get_mem_list_err:
		stc							; set the carry flag on error
		ret

mem_cpy: ; esi = src, edi = dest, ecx = size
	cmp ecx, 0
	je .done
	mov al, byte [esi]
	mov byte [edi], al
	inc esi
	inc edi
	dec ecx
	jmp mem_cpy

	.done:
		ret
