[bits 16]

%include "src/core/bios/macro.asm"

; returns
; al = video mode
; ah = number of char columns
BIOS_ROUTINE bios_get_video_mode
	mov ah, 0x0F
	int 0x10
	ret


; bh = Page Number, dh = Row, dl = Column
BIOS_ROUTINE bios_set_cursor_position
	mov ah, 0x02
	int 0x10
	ret

; bh = Page Number
; returns
; ax = 0
; ch = Start scan line
; cl = End scan line
; dh = Row
; dl = Column
BIOS_ROUTINE bios_get_cursor_position
	mov ah, 0x03
	int 0x10
	ret


dap:
dap_size: 		db 0x10
dap_reserved:	db 0x0
dap_sectors:	dw 0x1
dap_buf_off:	dw 0x0
dap_buf_seg:	dw 0x0
dap_lba:		dd 0x0
				dd 0x0
