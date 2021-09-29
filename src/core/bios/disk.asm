[bits 16]

%include "src/core/bios/macro.asm"

; ax = lba low
; bx = lba high
; cx = number of sectors (512 bytes)
; si = buffer offset
; di = buffer segment offset
; dl = drive number
BIOS_ROUTINE bios_read_sectors
	mov word [dap_lba], ax
	mov word [dap_lba + 2], bx
	mov word [dap_buf_off], si
	mov word [dap_buf_seg], di
	mov word [dap_sectors], cx
	
	mov ax, 0x4200
	mov si, dap
	int 0x13

	ret


dap:
dap_size: 		db 0x10
dap_reserved:	db 0x0
dap_sectors:	dw 0x1
dap_buf_off:	dw 0x0
dap_buf_seg:	dw 0x0
dap_lba:		dd 0x0
				dd 0x0
