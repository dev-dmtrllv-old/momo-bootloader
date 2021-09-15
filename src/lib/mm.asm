init_mm: ; si -> pointer to the bios memory map list, ax -> number of list items
	mov word [mem_map_items], ax		; copy the memory map items
	mov cx, 24							; 24 bytes per item
	mul cx								; multiply number of list items with 24 bytes = total bytes to copy
	mov cx, ax
	mov di, mem_map						; set copy destination
	rep movsb							; copy all bytes
	mov eax, mem_map
	ret

mm_get_mem_block: ; eax -> address, returns ecx = mem_map index
	xor ecx, ecx
	
	mov si, mem_map
	
	.find_loop:
		push ecx
		mov ebx, dword [si]				; base address
		mov edx, dword [si + 8]			; block size
		cmp ebx, eax
		jbe .base_below
		jmp .next

	.base_below:
		cmp edx, eax
		add ebx, edx
		jnc .not_overflowed
		mov ebx, 0xFFFFFFFF
	.not_overflowed:
		cmp ebx, eax
		jbe .next
		pop ecx
		clc
		ret

	.next:
		pop ecx
		mov bx, word [mem_map_items] 
		cmp bx, cx
		jbe .end
		inc ecx
		add si, 24
		jmp .find_loop

	.end:
		stc
		ret

mm_set_reserved: ; eax -> base address, ebx -> size
	push ebx
	push eax
	call mm_get_mem_block
	jc .mm_set_reserved_err
	xor eax, eax									; load the correct address from the pointer into esi and edi
	mov ax, 24
	mul cx											; ax is now the offset from the mem_map
	add eax, mem_map
	mov esi, eax
	mov edi, eax
	pop eax
	pop ebx
	
	mov ecx, dword [esi]							; cx points to blocks base
	mov edx, dword [esi + 8]						; dx points to blocks size

	cmp ebx, edx									; check if the requested size fits inside the blocks size
	jle .mm_set_reserved_fit
	
													; requested size does not fit within this block
	push eax										; check if the block is already used
	xor eax, eax
	mov ax, word [esi + 16]
	cmp ax, 0x2
	pop eax
	jge .mm_set_reserved_next
	cmp eax, ecx
	jge .mm_set_reserved_next_fit
	jmp .mm_set_reserved_err
	
	.mm_set_reserved_next_fit:
		mov word [esi + 16], 0x2					; set block as reserved

	.mm_set_reserved_next:							; set new (base, size) into (eax, ebx)
		add ecx, edx
		push ecx
		sub ecx, dword [esi]
		sub ebx, ecx
		pop eax
		call mm_set_reserved
		jnc .mm_set_reserved_done
		jmp .mm_set_reserved_err



	.mm_set_reserved_fit:
		push eax
		push ecx

		mov ax, word [esi + 16]						; check if the block is already used
		cmp ax, 0x2
		pop ecx
		pop eax

		jge .mm_set_reserved_done

		cmp eax, ecx								; check for the same base
		je .mm_set_reserved_equal_base

		push eax									; check for the same top address
		push ebx
		push ecx
		push edx

		add eax, ebx
		add ecx, edx
		cmp eax, ecx
		
		pop edx
		pop ecx
		pop ebx
		pop eax
		
		je .mm_set_reserved_equal_top

		; the block exists within the block boundaries (add 2 extra blocks)
		push eax
		sub eax, ecx
		mov dword [esi + 8], eax		; store the new size
		pop eax

		call mm_set_new_map_esi

		mov dword [esi], eax			; the base
		mov dword [esi + 8], ebx		; the size
		mov word [esi + 16], 2
		mov word [esi + 20], 1

		add eax, ebx					; calculate the max address of the reserved block
		add edx, ecx					; calculate the max of the block it is in
		sub edx, eax					; subtract the max's to get the size

		mov dword [esi + 24], eax		; the base
		mov dword [esi + 32], edx		; the size

		mov ax, word [edi + 16]			; copy flags from master block
		mov word [esi + 40], ax
		mov ax, word [edi + 20]			; copy flags from master block
		mov word [esi + 44], ax

		mov ax, word [mem_map_items]	; update the number of items in the list
		add ax, 2
		mov word [mem_map_items], ax

		jmp .mm_set_reserved_done
	
	.mm_set_reserved_equal_base:
		cmp ebx, edx
		je .mm_set_reserved_equal_base_and_size
		
		pusha
		add eax, ebx
		sub edx, ebx
		mov dword [esi], eax			; the base
		mov dword [esi + 8], edx		; the size
		mov dword [esi + 16], 1			; the size
		mov dword [esi + 20], 1			; the size
		popa

		call mm_set_new_map_esi

		mov dword [esi], eax
		mov dword [esi + 8], ebx		; the size
		mov word [esi + 16], 2			; set reserved flag
		mov word [esi + 20], 1			; set reserved flag
		
		mov ax, word [mem_map_items]	; update the number of items in the list
		inc ax
		mov word [mem_map_items], ax

		jmp .mm_set_reserved_done

	.mm_set_reserved_equal_top: 
		sub edx, ebx
		mov dword [esi + 8], edx		; subtract total size with requested size = new free size

		call mm_set_new_map_esi

		mov dword [esi], eax
		mov dword [esi + 8], ebx
		mov word [esi + 16], 2
		mov word [esi + 20], 1

		mov ax, word [mem_map_items]	; update the number of items in the list
		inc ax
		mov word [mem_map_items], ax
		
		jmp .mm_set_reserved_done
	

	.mm_set_reserved_equal_base_and_size:
		mov word [esi + 16], 2 
		mov word [esi + 20], 1
		
	.mm_set_reserved_done:
		clc
		ret
	
	.mm_set_reserved_err:
		stc
		ret


mm_free: ; eax -> base address

	ret

mm_set_new_map_esi:						; setsup the esi register to point to a new item in the mem_map array
	push eax
	push ecx
	push edx

	xor eax, eax
	mov ax, word [mem_map_items]
	mov ecx, 24
	mul ecx
	mov esi, mem_map
	add esi, eax

	pop edx
	pop ecx
	pop eax

	ret

mm_alloc: ; eax = size, returns eax = address
	xor ecx, ecx
	mov dx, word [mem_map_items]
	mov esi, mem_map

	.mm_alloc_loop:
		mov bx, word [esi + 16]	
		cmp bx, 1
		jne .mm_alloc_loop_next

		push eax
		mov ebx, dword [esi + 8]
		cmp eax, ebx
		pop eax
		jg .mm_alloc_loop_next
		mov ecx, eax
		mov eax, dword [esi]
		push eax
		mov ebx, ecx
		call mm_set_reserved
		pop eax
		ret


	.mm_alloc_loop_next:
		cmp cx, dx
		inc cx
		add esi, 24
		jne .mm_alloc_loop

	stc
	ret

mem_map_items:	dw 0
mem_map: 		TIMES 512 dd 0
