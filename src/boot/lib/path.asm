path_get_dir_count:	; esi = path string,  returns ecx = number of root directories to traverse
	push eax
	xor eax, eax
	xor ecx, ecx

	.get_char_loop:
		lodsb
		cmp al, '/'
		jne .skip
		inc ecx
		
	.skip:
		or al, al
		jz .done
		jmp .get_char_loop
	
	.done:
		pop eax
		ret

path_get_part: ; esi = pointer to path, ecx = path index, returns esi = pointer to buffer of the path part
	inc ecx
	xor ebx, ebx
	xor edx, edx
	mov edi, path_buffer
	.get_char_loop:
		lodsb
		cmp al, '/'
		jne .skip
		inc edx
		jmp .skip_add_char
	.skip:
		cmp edx, ecx
		jne .skip_add_char
		inc ebx
		cmp al, '.'
		jne .skip_write_ext
	.fill_till_ext:
		mov byte [edi], ' '	
		inc edi
		inc ebx
		cmp ebx, 8
		jle .fill_till_ext
		jmp .get_char_loop

	.skip_write_ext:
		mov [edi], al
		inc edi
		
	.skip_add_char:
		or al, al
		jz .done
		jmp .get_char_loop

	.done:
		mov byte [edi], ' '	
		inc edi
		inc ebx
		cmp ebx, 11
		jle .done
		
	mov esi, path_buffer
	call to_upper_case
	ret

to_upper_case:
	push esi 
	mov edi, esi
	mov cx, 11
	.check:
		lodsb
		inc edi
		or al, al
		je .done
		cmp al, 97
		jb .check
		cmp al, 122
		ja .check
		dec edi
		sub al, 32
		mov byte [edi], al
		inc edi
		jmp .check

	.done:

	pop esi
	ret

path_buffer:					times 11 dd 0, 0
