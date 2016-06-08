OUTPUT_FORMAT("elf32-sisa","elf32-sisa","elf32-sisa")
OUTPUT_ARCH(sisa)

ENTRY(_start)

SECTIONS
{
	/* Kernel code */
	. = 0xC000;
	.text : {
		_kernel_code_start = ABSOLUTE(.);
		*(.text.start)
		*(.text)
		*(.text*)
		*(.rodata)
		*(.rodata*)
		_kernel_code_end = ABSOLUTE(.);
	}

	/* Kernel data (data, bss and stack) */
	. = 0x8000;
	_kernel_data_start = ABSOLUTE(.);

	.data : {
		*(.data)
		*(.data*)
		/* FIXME: Remove this hack to force .data to exist */
		. += 4;
	}

	.bss : {
		_kernel_data_bss_start = ABSOLUTE(.);
		*(.bss)
		*(.bss*)
		_kernel_data_bss_end = ABSOLUTE(.);
	}
	/* Give at least 0x100 bytes to the stack */
	. += 0x100;
	. = ALIGN(0x1000);

	_kernel_data_end = ABSOLUTE(.);

	/* User */
	. = 0x1000;
	.user : {
		_user_code_start = ABSOLUTE(.);
		*(.user.code)
		_user_code_end = ABSOLUTE(.);
		. = ALIGN(0x1000);
		_user_data_start = ABSOLUTE(.);
		*(.user.data)
		_user_data_end = ABSOLUTE(.);
	}
}
