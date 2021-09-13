%include "lib/defines.asm"

[org BOOT1_ADDR]
[bits 16]

boot_start:
	jmp short boot_resume
	nop

oem_label				db "NOVABOOT"

; FAT12 (2.0)							  sector off:     BPB off:     size:		info:
bytes_per_sector 		dw 2				; 0x00B			0x00		WORD		Bytes per logical sector
sectors_per_cluster 	db 1 				; 0x00D			0x02		BYTE		Logical sectors per cluster
reserved_sectors		dw 20				; 0x00E			0x03		WORD		Reserved logical sectors
number_of_fats 			db 2 				; 0x010			0x05		BYTE		Number of FATs
root_dir_entries		dw 0 				; 0x011			0x06		WORD		Root directory entries
logical_sectors 		dw 6 				; 0x013			0x08		WORD		Total logical sectors (The total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume, and the actual count is stored in the Large Sector Count entry at 0x20.)
media_descriptor 		db 7 				; 0x015			0x0A		BYTE		Media descriptor
sectors_per_fat			dw 8 				; 0x016			0x0B		WORD		Logical sectors per FAT

; FAT12/FAT16 (3.31)
sectors_ter_track		dw 9				; 0x018			0x0D		WORD		Physical sectors per track (identical to DOS 3.0 BPB)
number_of_heads			dw 10				; 0x01A			0x0F		WORD		Number of heads (identical to DOS 3.0 BPB)
hidden_sectors 			dd 11				; 0x01C			0x11		DWORD		Hidden sectors (incompatible with DOS 3.0 BPB)
large_sector_count		dd 12 				; 0x020			0x15		DWORD		Large total logical sectors
 
; FOOTER INFO FOR FAT32 (7.1)
logical_sectors_per_fat	dd 1				; 0x024			0x19		DWORD		Logical sectors per FAT
mirroring_flags 		dw 1				; 0x028			0x1D		WORD		Mirroring flags etc.
version  				dw 1				; 0x02A			0x1F		WORD		Version
root_dir_cluster 		dd 1				; 0x02C			0x21		DWORD		Root directory cluster
fs_info_sector			dw 1				; 0x030			0x25		WORD		Location of FS Information Sector
backup_sector			dw 1				; 0x032			0x27		WORD		Location of backup sector(s)
boot_file_name 			db "BOOTFILENAME"	; 0x034			0x29		12 BYTES	Reserved (Boot file name)
drive 					db 1				; 0x040			0x35		BYTE		Physical drive number
bpb_flags 				db 1				; 0x041			0x36		BYTE		Flags etc.
ext_boot_sign			db 0x29 			; 0x042			0x37		BYTE		Extended boot signature (0x28)
volume_id				dd 0x3A623A48		; 0x043			0x38		DWORD		Volume serial number
volume_label			db "NOVAOS     "	; 0x047			0x3C		11 BYTES	Volume label
filesystem_type 		db "FAT32   "		; 0x052			0x47		8 BYTES		File-system type

times 20 db 0

boot_resume:
	mov [bpb_drive], dl						; store the booted drive number	

	mov cx, [reserved_sectors]				; load reserved sectors - 2 after the boot file system
	sub cx, 2
	mov word [dap_sectors], cx
	
	add ax, 2
	mov dword [dap_lba], eax
	mov word [dap_buf_off], 0x7E00
	call read_sectors
	jc read_err

	jmp 0x7E00

read_err:
	mov si, read_err_msg
	call print_line

	jmp halt

%include "lib/common.asm"
%include "lib/screen.asm"
%include "lib/disk.asm"

read_err_msg: 	db "Disk error!", 0

times 510 - ($-$$) db 0

dw 0xaa55
