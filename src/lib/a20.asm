check_a20:							; returns ax = 0 (a20 = disabled) or ax = 1 (a20 = enabled)
	pushf
	push ds
	push es
	push di
	push si
	
	cli

	xor ax, ax						; set [es:di] [0000:0500]
    mov es, ax
    mov di, 0x500

	xor ax, ax						; set [ds:si] [ffff:0510]
	not ax
	mov ds, ax
	mov si, 0x510

	mov al, byte [es:di]			; push the bytes to restore them later
    push ax
    mov al, byte [ds:si]
    push ax

	mov byte [es:di], 0x00				
    mov byte [ds:si], 0xff			
    cmp byte [es:di], 0xff			; check if the written value wrapped around

	pop ax							; restore the bytes
    mov byte [ds:si], al
    pop ax
    mov byte [es:di], al

	mov ax, 0						; default is diabled
	je check_a20_end				; if the bytes were the same we end
	mov ax, 1						; else we set ax to 1 and end

check_a20_end:
	pop si
	pop di
	pop es
	pop ds
	popf
	sti
	ret

enable_a20:							; returns ax = 0 (a20 = disabled) or ax = 1 (a20 = enabled)
	mov ax, 0x2401					; try enable a20 trhough bios
	int 0x15
	call check_a20
	cmp ax, 1
	je enable_a20_done

	cli
	call wait_cmd					; enable through keyboard controller
	mov al, 0xad					; send 0xad to disable the keyboard
	out 0x64, al

	call wait_cmd
	mov al, 0xd0					; send 0xd0 to read input
	out 0x64, al

	call wait_data
	in al, 0x60						; read input into al
	push ax						

	call wait_cmd
	mov al, 0xd1					; send 0xd1 to write to output
	out 0x64, al

	call wait_cmd
	pop ax
	or al, 2						; set bit 2
	out 0x60, al					; and write back to controller

	call wait_cmd
	mov al, 0xae					; send 0xae to enable keyboard
	out 0x64, al

	call wait_cmd
	sti

	call check_a20
	cmp ax, 1
	je enable_a20_done

	in al, 0x92						; try as last resort the fast gate
	or al, 2
	out 0x92, al

	call check_a20
	cmp ax, 1
	je enable_a20_done

enable_a20_done:
	ret

wait_cmd:
	in al, 0x64
	test al, 2
	jnz wait_cmd
	ret

wait_data:
	mov al, 0x64
	test al, 1
	jz wait_data
	ret
