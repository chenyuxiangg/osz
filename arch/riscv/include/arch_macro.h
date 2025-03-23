#ifndef __ARCH_MACRO_H__
#define __ARCH_MACRO_H__

#define REGBYTES                4

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

.macro  save_callee_reg
    sw   s0,    0 * REGBYTES(sp)
    sw   s1,    1 * REGBYTES(sp)
    sw   s2,    2 * REGBYTES(sp)
    sw   s3,    3 * REGBYTES(sp)
    sw   s4,    4 * REGBYTES(sp)
    sw   s5,    5 * REGBYTES(sp)
    sw   s6,    6 * REGBYTES(sp)
    sw   s7,    7 * REGBYTES(sp)
    sw   s8,    8 * REGBYTES(sp)
    sw   s9,    9 * REGBYTES(sp)
    sw   s10,   10 * REGBYTES(sp)
    sw   s11,   11 * REGBYTES(sp)
.endm

.macro  restore_callee_reg
    lw   s0,    0 * REGBYTES(sp)
    lw   s1,    1 * REGBYTES(sp)
    lw   s2,    2 * REGBYTES(sp)
    lw   s3,    3 * REGBYTES(sp)
    lw   s4,    4 * REGBYTES(sp)
    lw   s5,    5 * REGBYTES(sp)
    lw   s6,    6 * REGBYTES(sp)
    lw   s7,    7 * REGBYTES(sp)
    lw   s8,    8 * REGBYTES(sp)
    lw   s9,    9 * REGBYTES(sp)
    lw   s10,   10 * REGBYTES(sp)
    lw   s11,   11 * REGBYTES(sp)
.endm

.macro save_csr_reg
    csrr    t0, mepc
    csrr    t1, mstatus
    sw      t0, 12 * REGBYTES(sp)
    sw      t1, 13 * REGBYTES(sp)
.endm

.macro restore_csr_reg
    lw      t0, 12 * REGBYTES(sp)
    lw      t1, 13 * REGBYTES(sp)
    csrw    mepc, t0
    csrw    mstatus, t1
.endm

#endif