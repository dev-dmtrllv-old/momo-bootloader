halt:
	cli
	hlt
	jmp $

wait_shutdown:
	mov si, shutdown_msg
	call print_line

	.wait_for_enter:
    	mov ah, 0
    	int 0x16
    	cmp al, 0x0D
    	jne .wait_for_enter
		
shutdown:
	mov ax, 0x5307
	mov bx, 0x0001
	mov cx, 0x0003
	int 0x15
	jmp halt

shutdown_msg:		db "Press enter to shutdown...", 0
