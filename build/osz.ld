OUTPUT_ARCH("riscv")
OUTPUT_FORMAT("elf32-littleriscv")
ENTRY(_start)

MEMORY
{
	ITCM : ORIGIN = 0x80000000, LENGTH = 60M
	INIT : ORIGIN = 0x83c00000, LENGTH = 4M
	DTCM : ORIGIN = 0x84000000, LENGTH = 64M
}

PHDRS {
  text PT_LOAD FLAGS(5);  /* R-X (0x4 | 0x1 = 5) */
  data PT_LOAD FLAGS(6);  /* RW- (0x2 | 0x4 = 6) */
  init PT_LOAD FLAGS(5);
}

SECTIONS
{
	.text : {
		KEEP(*(.text.start))
		*(.text .text.*)
	} > ITCM :text

	.rodata ALIGN(0x20) : {
		*(.rodata .rodata.*)
	} > ITCM :text

	.rela ALIGN(0x20) : {
		*(.rela .rela.*)
	} > ITCM :text

	.init : {
		__init_start = .;
		__init_l0_start = .;
		KEEP(*(.init.init_l0))
		__init_l0_end = .;
		__init_l1_start = .;
		KEEP(*(.init.init_l1))
		__init_l1_end = .;
		__init_l2_start = .;
		KEEP(*(.init.init_l2))
		__init_l2_end = .;
		__init_l3_start = .;
		KEEP(*(.init.init_l3))
		__init_l3_end = .;
		__init_l4_start = .;
		KEEP(*(.init.init_l4))
		__init_l4_end = .;
		__init_end = .;
	} > INIT :init

	.data : {
		_data_start = .;
		*(.data .sdata .comment)
		_data_end = .;
	} > DTCM :data

	.got : {
		*(.got .got.*)
	} > DTCM :data

	. = ALIGN(0x20);
	.bss (NOLOAD) : {
		__system_bss_start = .;
		*(.bss .bss.*, .sbss)
		__system_bss_end = .;
	} > DTCM

	.stack (NOLOAD) : {
		. = ALIGN(0x20);
		*(.stack *.stack .stack.*)
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
