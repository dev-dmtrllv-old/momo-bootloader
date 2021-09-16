fat32_init: ; eax = file systems LBA
	mov [fat32_fs_lba], eax
	
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
	
	push eax
	xor ecx, ecx									; calculate cluster 0 lba
	mov cl, [bpb_sectors_per_cluster]
	mov eax, [bpb_root_dir_cluster]
	mul ecx
	pop ebx
	sub ebx, eax
	mov [fat32_start_lba], ebx

	xor eax, eax									; allocate some space for reading the file to before copying it to its destination
	mov ax, [bpb_bytes_per_sector]
	push eax
	call mm_alloc
	mov [file_buf_addr], eax
	pop eax											; allocate enough space to load 1 sector for the fat
	call mm_alloc
	mov [fat32_fat_buf_addr], eax

	xor eax, eax									; calculate the fat lba offset
	mov ax, word [bpb_reserved_sectors]
	mov ecx, [fat32_fs_lba]
	add eax, ecx
	mov [fat32_fat_lba], eax

	xor eax, eax
	mov ax, word [bpb_bytes_per_sector]
	xor ecx, ecx									; allocate enough space to load 1 cluster of the root dir
	mov cl, [bpb_sectors_per_cluster]
	mul ecx
	mov [fat32_cluster_size], eax
	call mm_alloc 
	mov [fat32_root_dir_addr], eax

	xor eax, eax									; calculate how many cluster numbers fit within one fat sector
	mov ax, [bpb_bytes_per_sector]
	mov ecx, 0x4
	xor edx, edx
	div ecx
	mov [cluster_indices_per_sector], eax

	ret


fat32_get_next_cluster: ; eax = cluster number to load fat table for , returns eax = new cluster number
	xor edx, edx									; find out which FAT sector to load based on a given cluster number, to find the next cluster number
	mov ecx, [cluster_indices_per_sector]
	div ecx
	
	push edx										; store the offset into the fat
	add eax, [fat32_fat_lba]
	
	cmp eax, [last_loaded_fat_sect]
	
	je .skip_load_fat_sect

	mov [last_loaded_fat_sect], eax

	mov dword [dap_lba], eax
	mov eax, [fat32_fat_buf_addr]
	mov [dap_buf_off], eax
	mov word [dap_buf_seg], 0
	mov word [dap_sectors], 1
	call read_sectors
	
	.skip_load_fat_sect:
		pop eax
		mov ecx, 4
		mul ecx
		mov edx, [fat32_fat_buf_addr]
		add edx, eax
		mov eax, [edx]
		ret


fat32_load_cluster: ; eax = cluster number, ebx = address where to load
	push ebx
	call fat32_cluster_to_lba
	mov [dap_lba], eax
	mov word [dap_sectors], 1
	mov word [dap_buf_seg], 0
	push eax
	mov eax, [file_buf_addr]
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
		push ecx
		xor ecx, ecx
		mov cx, [bpb_bytes_per_sector]				; bytes to copy
		mov esi, [file_buf_addr]					; from
		mov edi, ebx								; to
		add ebx, ecx
	
	.copy:											; copy 1 sector to bx
		mov al, byte [esi]
		mov byte [edi], al
		inc esi
		inc edi
		dec ecx
		cmp ecx, 0
		jne .copy

		pop ecx
		inc ecx
		inc dword [dap_lba]
		cmp ecx, edx
		jl .load_and_copy_sector
	
	.load_cluster_err:
		ret

fat32_get_path_info: ; esi = pointer to the path
	push esi
	call path_get_dir_count
	pop esi

	mov edx, ecx							; edx = number of path parts
	xor ecx, ecx

	mov eax, [bpb_root_dir_cluster]
	
	push esi

	.get_path_part:
		push ecx
		push edx
		push eax
		call path_get_part
		pop eax
		push eax

	.search_root_dir:
		push esi
		mov ebx, [fat32_root_dir_addr]
		call fat32_load_cluster
		pop esi
		call fat32_search_root_dir
		pop eax
		pop edx
		pop ecx
		jc .load_next_root_dir
		inc ecx
		cmp ecx, edx
		je .done
		; check if the found entry is a sub directory
		push eax
		mov al, [edi + 0x0B]
		cmp al, 0x10
		pop eax
		jne .search_failed	; if its not a sub dir return failed
		xor eax, eax
		mov ax, [edi + 0x14]
		shl eax, 0x10
		mov ax, [edi + 0x1A]
		pop esi
		push esi
		jmp .get_path_part

	.load_next_root_dir:
		push ecx
		push edx
		call fat32_get_next_cluster
		pop edx
		pop ecx
		cmp eax, EOC
		push ecx
		push edx
		push eax
		jl .search_root_dir
		pop eax
		pop edx
		pop ecx
	.search_failed:
		stc
		pop esi
		ret

	.done:
		clc
		pop esi
		ret

fat32_search_root_dir: ; esi = string to search for, returns edi = pointer to the found root dir entry, carry is set if not found
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
		clc
		ret


fat32_cluster_to_lba: ; eax = cluster number, return eax = lba
	xor ecx, ecx
	mov cl, [bpb_sectors_per_cluster]
	mul ecx
	mov ebx, [fat32_start_lba]
	add eax, ebx
	ret


fat32_load_from_entry: ; edi = pointer to entry
	xor eax, eax
	mov ax, [edi + 0x14]
	shl eax, 0x10
	mov ax, [edi + 0x1A]
	mov ebx, dword [edi + 0x1C]					; the size of the file in bytes

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
		cmp eax, EOC
		jae	.load_file_done
		jmp .load_cluster

	.load_file_done:
	pop eax											; restore the file pointer
	pop ebx
	ret


fat32_load_file: ; esi -> pointer to file path, 	returns eax = pointer to file, ebx = file size in bytes
	call fat32_get_path_info
	jc .file_not_found
	call fat32_load_from_entry
	clc
	jmp .done

	.file_not_found:
		stc
	
	.done:
		ret


fat32_release_mem:
	mov eax, [fat32_root_dir_addr]
	call mm_free
	mov eax, [fat32_fat_buf_addr]
	call mm_free
	mov eax, [file_buf_addr]
	call mm_free
	ret


cluster_indices_per_sector:		dd 0x0
fat32_start_lba:				dd 0x0
fat32_fat_lba: 					dd 0x0
fat32_fs_lba: 					dd 0x0
fat32_cluster_size:				dd 0x0
fat32_root_dir_entries:			dd 0x0
fat32_root_dir_addr:			dd 0x0
fat32_fat_buf_addr:				dd 0x0
file_buf_addr:					dd 0x0
last_loaded_fat_sect:			dd 0xFFFFFFFF
