OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(main)
SECTIONS
{
	. = 0x5000;
 
	.text BLOCK(4K) : ALIGN(4K)
	{
		code = .;
		text = .;
		*(.text.startup);
		*(.text);
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		rodata = .;
		*(.rodata);
	}
 
	.data BLOCK(4K) : ALIGN(4K)
	{
		data = .;
		*(.data);
	}
 
	.bss BLOCK(4K) : ALIGN(4K)
	{
		bss = .;
		*(COMMON);
		*(.bss);
	}

	/DISCARD/ : 
	{
		*(.eh_frame);
		*(.comment);
		*(.note*)
        *(.indent)
        *(.comment)
        *(.stab)
        *(.stabstr)
	}

    end = .;
}
