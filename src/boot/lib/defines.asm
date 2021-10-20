%define MBR_ADDR						0x7A00
%define BOOT1_ADDR						0x7C00
%define BOOT2_ADDR						0x1000
<<<<<<< HEAD:src/boot/lib/defines.asm

%define BOOT_INFO_ADDR					0x1000

%define CORE_LOAD_ADDR					0x2000
%define CORE_ENTRY_ADDR					0x10000
=======
%define CORE_ADDR						0x2000
>>>>>>> main:src/lib/defines.asm

%define PT_TABLE 						(MBR_ADDR + 0x1BE)
%define PT_ENTRY_SIZE					0x10

%define GDT_ADDR						0x800
%define PM_GDT_ADDR						0x900

%define EOC								0x0FFFFFF8

%ifndef MBR
%define bpb_oem_label					0x7C03
%define bpb_bytes_per_sector 	    	0x7C0B
%define bpb_sectors_per_cluster   		0x7C0D
%define bpb_reserved_sectors	    	0x7C0E
%define bpb_number_of_fats 	    		0x7C10
%define bpb_root_dir_entries	   		0x7C11
%define bpb_logical_sectors 	   		0x7C13
%define bpb_media_descriptor     		0x7C15
%define bpb_sectors_per_fat	    		0x7C16
%define bpb_sectors_ter_track			0x7C18
%define bpb_number_of_heads				0x7C1A
%define bpb_hidden_sectors 				0x7C1C
%define bpb_large_sector_count			0x7C20
%define bpb_logical_sectors_per_fat		0x7C24
%define bpb_mirroring_flags 			0x7C28
%define bpb_version  					0x7C2A
%define bpb_root_dir_cluster 			0x7C2C
%define bpb_fs_info_sector				0x7C30
%define bpb_backup_sector				0x7C32
%define bpb_boot_file_name 				0x7C34
%define bpb_drive 						0x7C40
%define bpb_bpb_flags 					0x7C41
%define bpb_ext_boot_sign				0x7C42
%define bpb_serial 						0x7C43
%define bpb_volume_ID					0x7C43
%define bpb_volume_label				0x7C47
%define bpb_filesystem_type 			0x7C52
%endif
