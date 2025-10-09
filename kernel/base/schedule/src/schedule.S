#include "arch_macro.h"

.section .text.init
#define MSTATUS_MIE     1 << 3
#define ALL_INTEGER_REG_SIZE    REG_SIZE*32

.global schedule
.balign 16

schedule:
    /* disable global interrupt */
    li t0, MSTATUS_MIE
    csrc mstatus, t0
    li t0, 0x1880
    csrs mstatus, t0

save_current_task:
    addi    sp, sp, -16 * REGBYTES
    save_caller_reg
    addi    sp, sp, -16 * REGBYTES
    save_callee_reg
    la      t0, g_is_positive_schechule
    lw      t0, 0(t0)
    beqz    t0, __not_change_mepc
    csrw    mepc, ra
__not_change_mepc:
    save_csr_reg

    la  t0, gp_current_task
    lw  t1, 0(t0)
    sw  sp, 0(t1)       /* save run task sp to tsk_context_pointer */

switch_to:
    call    switch_hook
    la t1, gp_new_task
    lw t1, 0(t1)
    sw t1, 0(t0)        /* save new task_cb to gp_current_task */
    lw sp, 0(t1)        /* restore new task sp from tsk_context_pointer */
    restore_csr_reg
    restore_callee_reg
    addi    sp, sp, 16 * REGBYTES
    restore_caller_reg
    addi    sp, sp, 16 * REGBYTES
    /* mret can set mstatus.mpie to mstatus.mie */
    mret

.global first_schedule
.balign 16
first_schedule:
    li t0, MSTATUS_MIE
    csrc mstatus, t0

    la  t0, gp_current_task
    j switch_to
