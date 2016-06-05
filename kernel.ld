OUTPUT_FORMAT("elf32-sisa","elf32-sisa","elf32-sisa")
OUTPUT_ARCH(sisa)

ENTRY(_start)

SECTIONS
{
	/* Kernel code */
	. = 0xC000;
	.text : { *(.text.start) *(.text) *(.rodata) *(.rodata*) }

	/* Kernel data */
	. = 0x8000;
	.data : { *(.data) *(.data*) *(.bss) *(.bss*) }

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
