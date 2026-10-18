OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

PHDRS
{
	ph_all PT_LOAD FLAGS (0x7); /* R+W+X — single segment, no overlap */
}

SECTIONS
{
	__payload_base = .;

	.text :
	{
		KEEP (*(.init))
		KEEP (*(.fini))

		*(.text .text.*)

		. = ALIGN(4);
	} : ph_all = 0x90909090

	.rodata :
	{
		*(.rodata .rodata.*)

		. = ALIGN(4);
	} : ph_all

	. = ALIGN(0x4000);

	.data :
	{
		*(.data .data.*)

		. = ALIGN(0x10);

		__imports_start = .;
		KEEP(*(.imports .imports.*))
		__imports_end = .;

		__patches_start = .;
		KEEP(*(.patches .patches.*))
		QUAD(0); BYTE(0); BYTE(0);
		__patches_end = .;

		__bss_start = .;
		*(.bss .bss.*) *(COMMON)
		__bss_end = .;

		. = . + 4;
		. = ALIGN(4);
	} : ph_all
}
