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
