OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(start)
SECTIONS
{

	__coreBinaryStart = .;
	
	. = 0x2000;
	
	.bios :
	{
		*(.bios)	
	}

	. = 0x10000;
	
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text.entry)
		*(.text.startup)
		*(.text.unlikely .text.*_unlikely .text.unlikely.*)
		*(.text.exit .text.exit.*)
		*(.text.hot .text.hot.*)
		*(SORT(.text.sorted.*))
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.text)
	}

	.preinit_array BLOCK(4K) : ALIGN(4K)
	{
	  	PROVIDE_HIDDEN (__preinit_array_start = .);
	  	KEEP (*(.preinit_array))
	  	PROVIDE_HIDDEN (__preinit_array_end = .);
	}
	
	.init_array BLOCK(4K) : ALIGN(4K)
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	}
	
	.fini_array BLOCK(4K) : ALIGN(4K)
	{
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	}
	
	.ctors BLOCK(4K) : ALIGN(4K)
	{
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	}
	
	.dtors BLOCK(4K) : ALIGN(4K)
	{
	  	KEEP (*crtbegin.o(.dtors))
	  	KEEP (*crtbegin?.o(.dtors))
	  	KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
	  	KEEP (*(SORT(.dtors.*)))
	  	KEEP (*(.dtors))
	}
	
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata);
		*(.gnu.linkonce.r*);
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data);
		*(.gnu.linkonce.d*);
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		sbss = .;
		*(COMMON);
		*(.bss);
		*(.gnu.linkonce.b*);
		ebss = .;
	}
	
	.ctors : {
		__ctors_beg = .;
		*(.ctors)
		__ctors_end = .;
	}	
	
	/DISCARD/ : 
	{
		*(.comment)
		*(.eh_frame)
	}	
	__coreBinaryEnd = .;

	end = .;
}
