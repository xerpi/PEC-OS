OUTPUT_FORMAT("elf32-sisa","elf32-sisa","elf32-sisa")
OUTPUT_ARCH(sisa)

ENTRY(_start)

SECTIONS
{
	. = 0x1000;
	.text : {
		*(.text.start)
		*(.text)
		*(.text*)
		*(.rodata)
		*(.rodata*)
	}

	. = ALIGN(0x1000);
	.data : {
		*(.data)
		*(.data*)
		*(.bss)
		*(.bss*)
		/* Give at least 0x100 bytes to the stack */
		. += 0x100;
		. = ALIGN(0x1000);
		_user_stack = ABSOLUTE(.);
	}
}
