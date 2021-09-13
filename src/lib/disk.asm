check_drive_ext:
	xor ax, ax
	xor dx, dx
	mov ah, 0x41
	mov dl, byte [bpb_drive]
	mov bx, 0x55aa
	int 0x13
	jc .drive_ext_err
	cmp bx, 0xaa55
	jnz .drive_ext_err
	ret

	.drive_ext_err:
		stc
		ret



read_sectors:
	pusha
	mov ax, 0x4200
	xor dx, dx
	mov ds, dx
	mov dl, byte [bpb_drive]
	mov si, dap
	int 0x13
	popa
	ret

dap:
dap_size: 		db 0x10
dap_reserved:	db 0x0
dap_sectors:	dw 0x1
dap_buf_off:	dw 0x0
dap_buf_seg:	dw 0x0
dap_lba:		dd 0x0
				dd 0x0
