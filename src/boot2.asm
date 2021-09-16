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
	mov sp, 0x7C00
	mov bp, sp
	mov [fs_lba], eax
	call clear_screen
	
	call check_a20
	cmp eax, 0
	jne a20_enabled
	call enable_a20
	cmp eax, 0
	jne a20_failed

a20_enabled:
	; copy the gdt to GDT_ADDR
	mov ecx, gdt_end
	mov ebx, gdtinfo
	sub ecx, ebx

	mov si, gdtinfo							; from
	mov di, GDT_ADDR						; to
	rep movsb

enter_unreal_mode:
	cli
	push ds

	lgdt [GDT_ADDR]

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
	mov [boot_info_mem_map_size], ebp			; store the number of list items

	mov esi, boot_info_mem_map
	mov eax, [boot_info_mem_map_size]
	call init_mm								; initialize the memory manager

	mov eax, 0x0								; set memory for IVT and GDT as reserved
	mov ebx, 0x1000
	call mm_set_reserved
	jc mm_set_reserved_err
	
	mov eax, 0x7A00								; add 0x200 bytes for the stack
	mov ebx, boot_info_mem_map
	sub ebx, eax
	call mm_set_reserved						; set the bootloader memory as reserved

	mov eax, CORE_ADDR - 8						; set memory reserved for the boot core
	mov ebx, 0x7A00								; (CORE_ADDR - 8 because we pass the pointer to the boot info struct at this address)
	sub ebx, CORE_ADDR - 8
	call mm_set_reserved

	mov eax, [fs_lba]							; initialize the FAT32 driver
	xor ebx, ebx
	mov bl, [bpb_drive]
	call fat32_init

	mov si, load_msg
	call print_line

	mov si, core_path
	call fat32_load_file
	jc file_not_found
	push eax
	mov [boot_info_core_size], ebx

	mov ecx, 0
	mov esi, eax
	mov edi, CORE_ADDR
	.copy_core:
		mov al, byte [esi]
		mov byte [edi], al
		inc esi
		inc edi
		dec ebx
		cmp ebx, 0
		jne .copy_core

	.remove_file_buf:	
		pop eax
		call mm_free

	mov si, config_path
	call fat32_load_file
	jc file_not_found

	mov [boot_info_config_size], ebx
	
	push eax
	mov eax, ebx
	call mm_alloc
	mov [boot_info_config_addr], eax
	mov ecx, ebx
	pop esi
	push esi
	mov edi, eax
	call mem_cpy
	pop eax
	call mm_free

	mov eax, 0x7A00
	call mm_free

	xor eax, eax
	mov ax, [mem_map_items]
	mov [boot_info_mem_map_size], eax
	mov dword [boot_info_mem_map], mem_map

	mov eax, CORE_ADDR - 8
	mov dword [eax], boot_info

	call fat32_release_mem

	jmp CORE_ADDR

error_handler file_not_found, file_not_found_msg
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
%include "lib/path.asm"

load_msg: 				db "Loading bootloader core...", 0
a20_fail_msg:			db "Failed to enable the A20 line!", 0
mem_list_err_msg:		db "Failed to get the memory list!", 0
set_reserved_err_msg:	db "Failed to set memory block as reserved!", 0
file_not_found_msg:		db "File not found!", 0

config_path:			db "/momo/boot.cfg", 0
core_path:				db "/momo/core.bin", 0

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

ALIGN 32

boot_info:
boot_info_core_size:	dd 0
boot_info_config_addr:	dd 0
boot_info_config_size:	dd 0
boot_info_mem_map_size:	dd 0
boot_info_mem_map: 		db 0


