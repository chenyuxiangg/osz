#include "menuconfig.h"
#include "trap_pri.h"

#define INTERRUPT_STACK_SIZE    OSZ_CFG_INTERRUPT_STACK_SIZE
#define EXCEPTION_STACK_SIZE    OSZ_CFG_EXCEPTION_STACK_SIZE
#define REGBYTES                4
#define INTERRUPT_MASK          0x80000000

.section .text.kernel.init
.globl  trap_entry
.globl  exception_handler
.globl  interrupt_handler

.macro  save_caller_reg
    sw   t6,    0 * REGBYTES(sp)
    sw   t5,    1 * REGBYTES(sp)
    sw   t4,    2 * REGBYTES(sp)
    sw   t3,    3 * REGBYTES(sp)
    sw   a7,    4 * REGBYTES(sp)
    sw   a6,    5 * REGBYTES(sp)
    sw   a5,    6 * REGBYTES(sp)
    sw   a4,    7 * REGBYTES(sp)
    sw   a3,    8 * REGBYTES(sp)
    sw   a2,    9 * REGBYTES(sp)
    sw   a1,   10 * REGBYTES(sp)
    sw   a0,   11 * REGBYTES(sp)
    sw   t2,   12 * REGBYTES(sp)
    sw   t1,   13 * REGBYTES(sp)
    sw   t0,   14 * REGBYTES(sp)
    sw   ra,   15 * REGBYTES(sp)
.endm

.macro  restore_caller_reg
    lw   t6,    0 * REGBYTES(sp)
    lw   t5,    1 * REGBYTES(sp)
    lw   t4,    2 * REGBYTES(sp)
    lw   t3,    3 * REGBYTES(sp)
    lw   a7,    4 * REGBYTES(sp)
    lw   a6,    5 * REGBYTES(sp)
    lw   a5,    6 * REGBYTES(sp)
    lw   a4,    7 * REGBYTES(sp)
    lw   a3,    8 * REGBYTES(sp)
    lw   a2,    9 * REGBYTES(sp)
    lw   a1,   10 * REGBYTES(sp)
    lw   a0,   11 * REGBYTES(sp)
    lw   t2,   12 * REGBYTES(sp)
    lw   t1,   13 * REGBYTES(sp)
    lw   t0,   14 * REGBYTES(sp)
    lw   ra,   15 * REGBYTES(sp)
.endm

.balign 4
trap_entry:
    save_caller_reg
    csrr    a0, mcause
    mv      s0, sp
    li      t0, INTERRUPT_MASK
    and     t0, a0, t0
    bne     t0, zero, __interrupt_entry
__exception_entry:
    la      sp, EXCEPTION_SP_TOP
    csrr    a1, mepc
    call    exception_handler
    csrw    mepc, a0
    j       __ret_task
__interrupt_entry:
    la      sp, INTERRUPT_SP_TOP
    call    interrupt_handler
__ret_task:
    mv      sp, s0
    restore_caller_reg
    mret

.balign 16
.section .stack.trap
.globl __interrupt_stack_top
.globl __exception_stack_top
__interrupt_stack:
    .space  INTERRUPT_STACK_SIZE
__interrupt_stack_top:

__exception_stack:
    .space  EXCEPTION_STACK_SIZE
__exception_stack_top: