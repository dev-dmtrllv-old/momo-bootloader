OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(start)
SECTIONS
{
	. = 0x2000;

	__coreBinaryStart = .;

	.text :
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
	
	.preinit_array    :
	{
	  	PROVIDE_HIDDEN (__preinit_array_start = .);
	  	KEEP (*(.preinit_array))
	  	PROVIDE_HIDDEN (__preinit_array_end = .);
	}
	
	.init_array    :
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	}
	
	.fini_array    :
	{
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	}
	
	.ctors :
	{
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	}
	
	.dtors :
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
		*(.eh_frame) /* discard this, unless you are implementing runtime support for C++ exceptions. */
	}	
	__coreBinaryEnd = .;

	end = .;
}
