/*
 * 最小 setjmp / longjmp（rv32）
 *
 * 只要能在同一任务内从用例函数任意位置跳回 runner 即可，
 * 因此只保存 callee-saved 寄存器、ra 与 sp，共 14 个 32 位字：
 *   [0] ra, [1] sp, [2..13] s0-s11
 *
 * int  tc_setjmp(tc_jmp_buf_t *buf);   // 首次返回 0
 * void tc_longjmp(tc_jmp_buf_t *buf);  // 只在 a0 里返回 1，调用方忽略
 */

    .text
    .align 2

    .globl tc_setjmp
    .type  tc_setjmp, @function
tc_setjmp:
    sw      ra,   0(a0)
    sw      sp,   4(a0)
    sw      s0,   8(a0)
    sw      s1,  12(a0)
    sw      s2,  16(a0)
    sw      s3,  20(a0)
    sw      s4,  24(a0)
    sw      s5,  28(a0)
    sw      s6,  32(a0)
    sw      s7,  36(a0)
    sw      s8,  40(a0)
    sw      s9,  44(a0)
    sw      s10, 48(a0)
    sw      s11, 52(a0)
    li      a0, 0
    ret
    .size  tc_setjmp, . - tc_setjmp

    .globl tc_longjmp
    .type  tc_longjmp, @function
tc_longjmp:
    lw      ra,   0(a0)
    lw      sp,   4(a0)
    lw      s0,   8(a0)
    lw      s1,  12(a0)
    lw      s2,  16(a0)
    lw      s3,  20(a0)
    lw      s4,  24(a0)
    lw      s5,  28(a0)
    lw      s6,  32(a0)
    lw      s7,  36(a0)
    lw      s8,  40(a0)
    lw      s9,  44(a0)
    lw      s10, 48(a0)
    lw      s11, 52(a0)
    li      a0, 1
    ret
    .size  tc_longjmp, . - tc_longjmp
