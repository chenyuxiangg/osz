OUTPUT_ARCH("riscv")
OUTPUT_FORMAT("elf32-littleriscv")
ENTRY(_start)

MEMORY
{
	ITCM : ORIGIN = 0x80000000, LENGTH = 64M
	DTCM : ORIGIN = 0x84000000, LENGTH = 64M
}

SECTIONS
{
	.text : {
		KEEP(*(.text.start))
		*(.text .text.*)
	} > ITCM

	.rodata ALIGN(0x20) : {
		*(.rodata .rodata.*)
	} > ITCM

	.rela ALIGN(0x20) : {
		*(.rela .rela.*)
	} > ITCM

	.data : {
		_data_start = .;
		*(.data .sdata .comment)
		_data_end = .;
	} > DTCM

	. = ALIGN(0x20);
	.bss (NOLOAD) : {
		__system_bss_start = .;
		*(.bss .bss.*, .sbss)
		__system_bss_end = .;
	} > DTCM

	.stack (NOLOAD) : {
		*(.stack *.stack)
	} > DTCM

	.heap (NOLOAD) : {
		. = ALIGN(0x20);
		__system_heap_start = .;
		__system_heap_end = ORIGIN(DTCM) + LENGTH(DTCM);
	} > DTCM

	/DISCARD/ : {
		*(.riscv .riscv.* .note.*)
	}

	__system_heap_size = __system_heap_end - __system_heap_start;
	_end = . ;
}
