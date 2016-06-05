OUTPUT_FORMAT("elf32-sisa","elf32-sisa","elf32-sisa")
OUTPUT_ARCH(sisa)

ENTRY(_start)

SECTIONS
{
	. = 0x1000;
	.text : { *(.text.start) *(.text) *(.rodata) *(.rodata*) }
	. = ALIGN(0x1000);
	.data : { *(.data) *(.data*) *(.bss) *(.bss*) }
}
