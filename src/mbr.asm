%define MBR
%include "lib/defines.asm"
%undef MBR

[org MBR_ADDR]
[bits 16]

start:
	cli
	xor ax, ax								; zero out all registers
	mov ds, ax
	mov es, ax
	mov ss, ax								; setup the stack	
	mov ax, 0x7A00
   	mov sp, ax	 							; stack pointer (grows downwards from 0x7c00 -> 0x0000)
	cld										; clear direction flag
	
	mov cx, 0x100							; copy 256 words (512 bytes)
	mov si, 0x7C00							; from
	mov di, 0x7A00							; to
	rep movsw
	jmp 0:relocated_start

relocated_start:
	mov [bpb_drive], dl							; set the drive number

	call check_drive_ext
	jc drive_ext_fail

	mov si, PT_TABLE
	mov cx, 4								; loop over the 4 entries
find_active_loop:
	mov ax, [si]
	cmp ax, 0x80
	je found_active_partition
	add si, PT_ENTRY_SIZE
	loop find_active_loop
	jmp found_no_active_partition

found_active_partition:
	add si, 0x8
	mov eax, [si]
	push eax
	mov dword [dap_lba], eax
	mov word [dap_buf_off], 0x7C00
	call read_sectors
	jc read_sectors_err
	mov dl, [bpb_drive]
	pop eax
	jmp 0x7C00

found_no_active_partition:
	mov si, found_no_active_msg
	call print_line
	jmp halt
	
read_sectors_err:
	mov si, disk_err_msg
	call print_line
	jmp halt

drive_ext_fail:
	mov si, drive_ext_err_msg
	call print_line
	jmp halt

bpb_drive:				db 0
found_no_active_msg:	db "No bootable partition found!", 0
disk_err_msg:			db "Disk error!", 0
drive_ext_err_msg:		db "Drive extension not available!", 0

%include "lib/common.asm"
%include "lib/screen.asm"
%include "lib/disk.asm"


