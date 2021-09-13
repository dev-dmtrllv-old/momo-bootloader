%include "lib/defines.asm"

%macro error_handler 2
%1:
	mov si, %2
	call print_line
	jmp wait_shutdown
%endmacro

[org BOOT2_ADDR]
[bits 16]

boot_start:
	mov [fs_lba], eax
	call clear_screen
	
	call check_a20
	cmp eax, 0
	jne a20_enabled
	call enable_a20
	cmp eax, 0
	jne a20_failed

a20_enabled:
	mov si, load_msg
	call print_line

enter_unreal_mode:
	cli
	push ds

	lgdt [gdtinfo]

	mov  eax, cr0								; switch to pmode by
   	or al, 1									; set pmode bit
   	mov  cr0, eax
	jmp $ + 2									; tell 386/486 to not crash
	
	mov bx, 0x08								; select descriptor 1
	mov ds, bx									; 8h = 1000b

	and al, 0xFE								; back to realmode
   	mov cr0, eax								; by toggling bit again

	pop ds
	sti

	xor eax, eax								; get the memory map from the bios
	mov es, eax
	mov di, boot_info_mem_map
	call get_mem_list
	jc get_mem_list_err
	mov word [boot_info_mem_map_size], bp		; store the number of list items

	mov si, boot_info_mem_map
	mov ax, word [boot_info_mem_map_size]
	call init_mm								; initialize the memory manager

	mov eax, 0x0								; set memory for IVT and GDT as reserved
	mov ebx, 0x800
	call mm_set_reserved
	jc mm_set_reserved_err

	xor ecx, ecx								; set the bootloader memory as reserved
	mov cx, [boot_info_mem_map_size]
	add eax, ecx
	mov eax, 24
	mul ecx 
	add eax, boot_info_mem_map
	mov ebx, boot_start
	xchg ebx, eax
	sub ebx, eax
	call mm_set_reserved
	jc mm_set_reserved_err

	mov eax, [fs_lba]							; initialize the FAT32 driver
	xor ebx, ebx
	mov bl, [bpb_drive]
	call fat32_init

	jmp halt

	jmp wait_shutdown




error_handler a20_failed, a20_fail_msg
error_handler get_mem_list_err, mem_list_err_msg
error_handler mm_set_reserved_err, set_reserved_err_msg

%include "lib/common.asm"
%include "lib/screen.asm"
%include "lib/disk.asm"
%include "lib/a20.asm"
%include "lib/mem.asm"
%include "lib/mm.asm"
%include "lib/fat32.asm"

load_msg: 				db "Loading bootloader core...", 0
a20_fail_msg:			db "Failed to enable the A20 line!", 0
mem_list_err_msg:		db "Failed to get the memory list!", 0
set_reserved_err_msg:	db "Failed to set memory block as reserved!", 0
gdtinfo:
	dw gdt_end - gdt_desc - 1				; last byte in table
	dd gdt_desc								; start of table

gdt_desc:
	dd 0
	dd 0

flat_desc:
	db 0xff
	db 0xff
	db 0
	db 0
	db 0
	db 10010010b
	db 11001111b
	db 0

gdt_end:

fs_lba: dd 0

boot_info:
boot_info_mem_map_size:	dw 0
boot_info_mem_map: 		db 0
