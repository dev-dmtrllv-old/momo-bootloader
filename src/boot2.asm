%include "lib/defines.asm"

%macro error_handler 2
%1:
	mov si, %2
	call print_line
	jmp wait_shutdown
%endmacro

%macro msg 2
%1:
	mov si, %2
	call print_line
	ret
%endmacro


[org BOOT2_ADDR]
[bits 16]

boot_start:
	mov sp, BOOT2_ADDR
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
	call print_a20_enabled

	mov ecx, gdt_end							; copy the gdt to GDT_ADDR (just above the IVT from the bios)
	mov ebx, gdt_info
	sub ecx, ebx

	mov si, gdt_info							; from
	mov di, GDT_ADDR							; to
	call mem_cpy

enter_unreal_mode:
	cli
	push ds
	
	xor ax, ax
	mov ds, ax

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

unreal_world_entered:
	; initialize fat32 variables
	mov eax, [fs_lba]							; calculate the FAT LBA
	xor edx, edx
	mov dx, word [bpb_reserved_sectors]
	add eax, edx
	mov dword [fat_lba], eax
	
	push eax									; calculate the Root Directory LBA
	xor ecx, ecx
	mov eax, dword [bpb_logical_sectors_per_fat]
	mov cl, [bpb_number_of_fats]
	mul ecx
	pop edx
	add eax, edx
	mov dword [root_dir_lba], eax

	push eax
	xor ecx, ecx									; calculate cluster 0 lba
	mov cl, [bpb_sectors_per_cluster]
	mov eax, [bpb_root_dir_cluster]
	mul ecx
	pop ebx
	sub ebx, eax
	mov dword [data_lba], ebx
	
	xor eax, eax								; store number of sectors per cluster
	mov al, byte [bpb_sectors_per_cluster]	
	mov dword [sectors_per_cluster], eax

	xor eax, eax								; store bytes per sector
	mov ax, word [bpb_bytes_per_sector]
	mov dword [bytes_per_sector], eax

	xor edx, edx								; store number of cluster numbers that fits within one sector
	mov ecx, 0x4
	div ecx
	mov [cluster_indices_per_sector], eax

	mov eax, dword [bpb_root_dir_cluster]		; store first root dir cluster number
	mov dword [root_dir_cluster_num], eax

	pusha
	call load_core
	popa

	; load in the first sector of the root dir and scan till we found the core dir entry
	; eax = root dir cluster number

	
	mov ebx, core_dir_name
	mov esi, [root_dir_lba]

load_dir_entry: ; eax = roor dir cluster number, ebx = entry string to match
	push eax
	call cluster_to_lba
	mov esi, eax
	mov ecx, [sectors_per_cluster]

	.load_root_dir_sectors:
		test ecx, ecx
		jz .load_next_root_dir_cluster	; if all the root dir cluster sectors have been searched load the next root dir cluster
		jmp .load_root_dir				
	.load_next_root_dir_cluster:		; get the next root dir cluster number
		pop eax
		call get_next_cluster
		push eax

		cmp eax, EOC					; check if the cluster is end of cluster
		jae file_not_found				; if so tell the user we couldn't find the core image
		call cluster_to_lba
		mov esi, eax

	.load_root_dir:
		push esi
		push ecx

		mov [dap_lba], esi
		mov dword [dap_buf_off], fs_buffer
		call read_sectors

		mov esi, ebx
	
	.search_root_dir:
		push esi
		mov ecx, 16
		mov edi, fs_buffer

	.find_file_loop:
		push edi
		push ecx
		push esi
		mov ecx, 11									; check 11 characters
		rep cmpsb
		pop esi
		pop ecx
		pop edi

		je .file_entry_found

		add di, 0x20								; point to the next entry (di + 32 bytes)

		test ecx, ecx								; check if end of root dir
		dec ecx
		jnz .find_file_loop

		pop esi										; file not found
		pop ecx
		pop esi

		inc esi
		dec ecx
		
		jmp .load_root_dir_sectors					; load next root dir sector

	.file_entry_found:
		pop ecx
		pop esi
		cmp ebx, core_dir_name						; check if we were looking for the momo root dir entry
		jne .core_image_found

		xor eax, eax								; load the cluster number for the next root dir
		mov ax, [edi + 0x14]
		shl eax, 0x10
		mov ax, [edi + 0x1A]
		mov ebx, core_file_name
		jmp load_dir_entry

	.core_image_found:								; core image is found!
		xor eax, eax								; lets get the cluster number
		mov ax, [edi + 0x14]
		shl eax, 0x10
		mov ax, [edi + 0x1A]

	mov edi, CORE_ADDR
	.load_file_cluster:								; eax = file cluster number, edi = where to load to
		mov ecx, dword [sectors_per_cluster]
		push eax
		call cluster_to_lba
	.load_file_cluster_loop:
		test ecx, ecx
		jz .load_cluster_loop_done
		mov [dap_lba], eax
		mov [dap_buf_off], edi
		call read_sectors
		add edi, [bytes_per_sector]
		dec ecx
		inc eax
		jmp .load_file_cluster_loop
	.load_cluster_loop_done:
		pop eax
		call get_next_cluster
		cmp eax, EOC								; check if the cluster is end of cluster
		jb .load_file_cluster						; if not load the next cluster

	jmp CORE_ADDR



get_next_cluster: ; eax = cluster number, returns eax = new cluster number
	; calculate which fat sector to load
	push ebx
	push ecx
	push edx
	xor edx, edx
	mov ecx, [cluster_indices_per_sector]
	div ecx
	push edx							; edx contains the remainder
	; load the fat sector
	add eax, dword [fat_lba]
	mov dword [dap_lba], eax
	mov dword [dap_buf_off], fs_buffer
	call read_sectors
	; calculate the offset into the fat table
	pop eax
	mov ecx, 4
	mul ecx
	mov edx, fs_buffer
	add edx, eax
	mov eax, dword [edx]
	; return with the new cluster number in eax
	pop edx
	pop ecx
	pop ebx
	ret

cluster_to_lba: ; eax = cluster number, returns eax = lba number
	push edx
	push ecx
	mov ecx, [sectors_per_cluster]
	mul ecx
	mov edx, [data_lba]
	add eax, edx
	pop ecx
	pop edx
	ret


msg print_a20_enabled, a20_enabled_msg
msg load_core, load_msg

error_handler file_not_found, file_not_found_msg
error_handler a20_failed, a20_fail_msg
error_handler get_mem_list_err, mem_list_err_msg

%include "lib/common.asm"
%include "lib/screen.asm"
%include "lib/disk.asm"
%include "lib/a20.asm"
%include "lib/mem.asm"

load_msg: 				db "Loading bootloader core...", 0
a20_enabled_msg: 		db "A20 enabled...", 0
a20_fail_msg:			db "Failed to enable the A20 line!", 0
mem_list_err_msg:		db "Failed to get the memory list!", 0
file_not_found_msg:		db "Couldn't find the core image at /momo/core.bin!", 0

core_dir_name:			db "MOMO       ", 0
core_file_name:			db "CORE    BIN", 0

gdt_info:
	dw gdt_end - gdt_zero_desc - 1			; last byte in table
	dd GDT_ADDR + 6							; start of table

gdt_zero_desc:
	dd 0
	dd 0

flat_desc:
	dw 0xffff								; limit 0 : 15
	dw 0									; base 0 : 15
	db 0									; base 16 : 23
	db 10011010b							; access byte
	db 11001111b							; flags + limit [f f f f l l l l]
	db 0									; base 24 : 31

gdt_end:

fs_lba: 						dd 0
fat_lba: 						dd 0
root_dir_lba: 					dd 0
root_dir_cluster_num:			dd 0
data_lba:						dd 0
sectors_per_cluster:			dd 0
bytes_per_sector:				dd 0
cluster_indices_per_sector: 	dd 0

ALIGN 4
fs_buffer:
