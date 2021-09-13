fat32_init: ; eax = fs start lba on the disk, ebx = drive number
	mov [fat32_fs_lba], eax
	mov [fat32_drive_num], ebx

	xor eax, eax
	xor ecx, ecx	
	mov al, byte [bpb_number_of_fats]				; calculate the directory table offset ((fat table size * num of tat tables) + num of reserved sectors)
	mov ecx, dword [bpb_logical_sectors_per_fat]
	mul ecx 										; fat tables * sectors per fat
	add ax, word [bpb_reserved_sectors]				; add reserved sectors
	xor ecx, ecx
	xor edx, edx
	mov cl, [bpb_sectors_per_cluster]
	add eax, [fat32_fs_lba]							; add the file system start sector
	push eax										; push the root dir lba
	
	xor ecx, ecx									; calculate cluster 0 lba
	mov cl, [bpb_sectors_per_cluster]
	mov eax, [bpb_root_dir_cluster]
	mul ecx
	pop ebx
	sub ebx, eax
	mov [fat32_start_lba], ebx

	xor eax, eax
	mov ax, [bpb_bytes_per_sector]
	push eax
	call mm_alloc
	mov [file_buf], eax								; allocate some space for reading the file to before copying it to its destination
	pop eax
	
	xor ecx, ecx
	mov cl, [bpb_sectors_per_cluster]
	mul ecx
	push eax
	mov [fat32_cluster_size], eax
	call mm_alloc 
	mov [fat32_root_dir_addr], eax					; allocate enough space to load the root dir to
	pop eax
	xor edx, edx
	mov ecx, 0x20									; calculate number of root dir entries
	div ecx
	mov [fat32_root_dir_entries], eax

	mov ebx, [fat32_root_dir_addr]					; load in the FAT 32 Root Directory
	mov eax, [bpb_root_dir_cluster]
	call fat32_load_cluster

	xor eax, eax									; calculate the fat lba offset
	mov ax, word [bpb_reserved_sectors]
	mov ecx, [fat32_fs_lba]
	add eax, ecx
	mov [fat32_fat_lba], eax

	xor eax, eax									; allocate enough space to load 1 sector for the fat
	mov ax, word [bpb_bytes_per_sector]
	call mm_alloc
	mov [fat_buf_addr], eax

	xor eax, eax
	mov ax, [bpb_bytes_per_sector]
	mov ecx, 0x4
	xor edx, edx
	div ecx
	mov [cluster_numbers_per_sector], eax

	ret


fat32_load_cluster: ; eax = cluster number, ebx = address where to load
	push ebx
	call fat32_cluster_to_lba
	mov [dap_lba], eax
	mov word [dap_sectors], 1
	mov word [dap_buf_seg], 0
	push eax
	mov eax, [file_buf]
	mov [dap_buf_off], eax
	pop eax
	pop ebx
	xor dx, dx
	mov dl, [bpb_sectors_per_cluster]
	mov cx, 0
	
	.load_and_copy_sector:
		push ebx
		call read_sectors
		pop ebx
		jc .load_cluster_err
		push cx
		mov cx, [bpb_bytes_per_sector]				; bytes to copy
		mov esi, [file_buf]							; from
		mov di, bx									; to
		add bx, cx
		rep movsb									; copy 1 whole sector to bx
		pop cx
		inc cx
		inc dword [dap_lba]
		cmp cx, dx
		jl .load_and_copy_sector
	
	.load_cluster_err:
		ret



fat32_cluster_to_lba: ; eax = cluster number, return eax = lba
	xor ecx, ecx
	mov cl, [bpb_sectors_per_cluster]
	mul ecx
	mov ebx, [fat32_start_lba]
	add eax, ebx
	ret


fat32_load_file: ; esi = pointer to file name string, returns eax = pointer to start of file, ebx = file size in bytes
	call fat32_find_file_entry
	jc .load_file_not_found
	push ebx
	push eax
	mov ecx, [fat32_cluster_size]					; calculate file size aligned to cluster size
	mov eax, ebx
	xor edx, edx
	div ecx
	cmp edx, 0
	je .skip_add_cluster
	inc eax
	.skip_add_cluster:
	mul ecx
	call mm_alloc
	pop ebx											; get the cluster number in ebx
	xchg eax, ebx
	push ebx										; store the file pointer
	
	.load_cluster:
		push eax
		call fat32_load_cluster
		pop eax
		call fat32_get_next_cluster
		cmp eax, 0x0FFFFFF8
		jae	.load_file_done
		jmp .load_cluster

	.load_file_done:
	pop eax											; restore the file pointer
	pop ebx
	ret

	.load_file_not_found:
		stc
		ret


fat32_find_file_entry: ; returns edi = root dir entry, eax = cluster number, ebx = file size
	push esi
	
	mov ecx, [fat32_root_dir_entries]
	mov edi, [fat32_root_dir_addr]

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

		cmp cx, 0									; check if end of root dir
		dec cx
		jne .find_file_loop

		pop esi										; file is not found, set carry and return
		stc
		ret

	.file_entry_found:
		pop esi
		xor eax, eax
		mov ax, [edi + 0x14]
		shl eax, 0x10
		mov ax, [edi + 0x1A]
		mov ebx, dword [edi + 0x1C]					; the size of the file in bytes
		clc
		ret

	
fat32_get_next_cluster: ; returns eax = new cluster number
		
	xor edx, edx									; find out which FAT sector to load based on a given cluster number, to find the next cluster number
	mov ecx, [cluster_numbers_per_sector]
	div ecx
	push edx										; store the offset into the fat
	add eax, [fat32_fat_lba]
	
	cmp eax, [last_loaded_fat_sect]
	je .skip_load_fat_sect

	mov [last_loaded_fat_sect], eax

	mov dword [dap_lba], eax
	mov eax, [fat_buf_addr]
	mov [dap_buf_off], eax
	mov word [dap_buf_seg], 0
	mov word [dap_sectors], 1
	call read_sectors
	
	.skip_load_fat_sect:
		pop eax
		mov ecx, 4
		mul ecx
		mov edx, [fat_buf_addr]
		add edx, eax
		mov eax, [edx]
	ret

cluster_numbers_per_sector:		dd 0x0
file_buf:						dd 0x0
fat32_cluster_size:				dd 0x0
fat32_root_dir_addr:			dd 0x0
fat32_root_dir_entries:			dd 0x0
fat32_fat_lba:					dd 0x0
fat32_fs_lba:					dd 0x0				; the lba for sector 0 (BPB, bootloader etc)
fat32_drive_num:				dd 0x0
fat32_start_lba:				dd 0x0				; is the lba for cluster 0
fat_buf_addr:					dd 0x0
last_loaded_fat_sect:			dd 0xFFFFFFFF
