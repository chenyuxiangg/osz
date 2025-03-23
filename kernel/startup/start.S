#include "menuconfig.h"
#define STARTUP_STACK_SIZE      OSZ_CFG_STARTUP_STACK_SIZE

.section .text.start
.globl _start
.globl trap_entry

.macro ZERO_REG reg
	xor		\reg, \reg, \reg
.endm
 
_start:
	csrr	t0, mhartid
	bnez	t0, hold

init_reg:
	ZERO_REG x1
	ZERO_REG x2
	ZERO_REG x3
	ZERO_REG x4
	ZERO_REG x5
	ZERO_REG x6
	ZERO_REG x7
	ZERO_REG x8
	ZERO_REG x9
	ZERO_REG x10
	ZERO_REG x11
	ZERO_REG x12
	ZERO_REG x13
	ZERO_REG x14
	ZERO_REG x15
	ZERO_REG x16
	ZERO_REG x17
	ZERO_REG x18
	ZERO_REG x19
	ZERO_REG x20
	ZERO_REG x21
	ZERO_REG x22
	ZERO_REG x23
	ZERO_REG x24
	ZERO_REG x25
	ZERO_REG x26
	ZERO_REG x27
	ZERO_REG x28
	ZERO_REG x29
	ZERO_REG x30
	ZERO_REG x31
	
reset:
	la		t0, trap_entry
	csrw	mtvec, t0
	
	la 		t0, __startup_stack_top
	mv		sp, t0

init_bss:
	la 		a0, __system_bss_start
	la		a1, __system_bss_end
	call	clean_bss
	j		os_init

clean_bss:
	addi	sp, sp, -4
	sw		ra,	0(sp)
	beqz	a1, _clean_end
	bgt		a0, a1, _clean_end
_clean_loop:
	sw		zero, 0(a0)
	addi	a0, a0, 4
	blt		a0, a1, _clean_loop
_clean_end:
	lw		ra, 0(sp)
	addi	sp, sp, 4
	ret

hold:
	wfi
	j	hold

.balign 16
.section .stack
__startup_stack:
    .space  STARTUP_STACK_SIZE
__startup_stack_top: