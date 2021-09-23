OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(start)
SECTIONS
{
	. = 0x3000;
	
	__coreBinaryStart = .;
 
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text.init);
		*(.text.startup);
		text = .;
		*(.text);
		*(.gnu.linkonce.t*); /* Used by gcc for Vague Linkage */
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		start_ctors = .;
        *(SORT(.ctors*))  /* Note the "SORT" */
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

		rodata = .;
		*(.rodata);
		*(.gnu.linkonce.r*);
	}
 
	.data BLOCK(4K) : ALIGN(4K)
	{
		data = .;
		*(.data);
		*(.gnu.linkonce.d*);
	}
 
	.bss BLOCK(4K) : ALIGN(4K)
	{
		sbss = .;
		bss = .;
		*(COMMON);
		*(.bss);
		*(.gnu.linkonce.b*);
		ebss = .;
	}

	/DISCARD/ : 
	{
		*(.comment)
        *(.eh_frame) /* discard this, unless you are implementing runtime support for C++ exceptions. */
	}

	__coreBinaryEnd = .;
	
    end = .;
}
