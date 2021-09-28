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
	xor ebx, ebx
	mov bl, dl
	mov [drive_number], ebx

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
	cli

	mov eax, cr0
	or al, 1
	mov cr0, eax

	lgdt [gdt_ptr]
	
	jmp 0x8:pm_entry


msg print_a20_enabled, a20_enabled_msg
error_handler a20_failed, a20_fail_msg

a20_enabled_msg: 	db "A20 enabled...", 0
a20_fail_msg: 		db "A20 failed...", 0

%include "lib/common.asm"
%include "lib/screen.asm"
%include "lib/a20.asm"


align 4

[bits 32]
pm_entry:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov sp, BOOT2_ADDR

	cld

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
		call pm_read_sector

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

		mov edi, CORE_LOAD_ADDR
	.load_file_cluster:								; eax = file cluster number, edi = where to load to
		mov ecx, dword [sectors_per_cluster]
		push eax
		call cluster_to_lba
	.load_file_cluster_loop:
		test ecx, ecx
		jz .load_cluster_loop_done
		mov [dap_lba], eax
		mov word [dap_buf_off], fs_buffer
		call pm_read_sector
	
		; copy here.....
		push esi
		push ecx

		mov ecx, 128
		mov esi, fs_buffer
		rep movsd
		
		pop ecx
		pop esi
		
		dec ecx
		inc eax
		jmp .load_file_cluster_loop
	.load_cluster_loop_done:
		pop eax
		call get_next_cluster
		cmp eax, EOC
		jae load_file_done
		jmp .load_file_cluster

load_file_done:
	mov ebp, 0x2000 - 8
	mov esp, ebp

	jmp CORE_ENTRY_ADDR

	hlt
	jmp $

file_not_found:
	mov eax, 0x12341234
	mov ebx, 0x12341234
	mov ecx, 0x12341234
	mov edx, 0x12341234
	
	hlt
	jmp $
	



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
	mov word [dap_buf_off], fs_buffer
	call pm_read_sector
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





pm_read_sector:
	cli
	pusha

	mov dword [sp_ptr], esp
	mov dword [bp_ptr], ebp

	jmp word 0x18:pm_16
[bits 16]
pm_16:
	mov ax, 0x20
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov eax, cr0
    and al, ~0x01
    mov cr0, eax

	jmp word 0x0:rm_16

rm_16:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	sti
	mov ax, 0x4200
	xor dx, dx
	mov ds, dx
	mov dl, byte [drive_number]
	mov si, dap
	int 0x13
	cli

	mov eax, cr0
	or al, 1
	mov cr0, eax

	jmp 0x8:pm_read_sector_done

align 4

[bits 32]
pm_read_sector_done:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	cld

	mov ebp, dword [sp_ptr]
	mov esp, ebp
	mov ebp, dword [bp_ptr]

	popa
	
	ret


core_dir_name:			db "MOMO       ", 0
core_file_name:			db "CORE    BIN", 0

drive_number:					dd 0
fs_lba: 						dd 0
fat_lba: 						dd 0
root_dir_lba: 					dd 0
root_dir_cluster_num:			dd 0
data_lba:						dd 0
sectors_per_cluster:			dd 0
bytes_per_sector:				dd 0
cluster_indices_per_sector: 	dd 0

%include "lib/dap.asm"

sp_ptr: dd 0x00000000
bp_ptr: dd 0x00000000

idt16_ptr:
    dw 0x03FF
    dd 0x0
     
gdt_base:
    .null:
        dd 0x0
        dd 0x0
         
    .code32:
        dw 0xFFFF
        dw 0x0
        db 0x00
        db 0x9A
        db 0xCF
        db 0x00
         
    .data32:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 0x92
        db 0xCF
        db 0x0
         
    .code16:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 0x9A
        db 0x0F
        db 0x0
         
    .data16:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 0x92
        db 0x0F
        db 0x0
         
gdt_ptr:
    dw gdt_ptr - gdt_base - 1
    dd gdt_base

ALIGN 4
fs_buffer:
