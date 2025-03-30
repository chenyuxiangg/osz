#include "menuconfig.h"
#include "trap_pri.h"
#include "arch_macro.h"

#define INTERRUPT_STACK_SIZE    OSZ_CFG_INTERRUPT_STACK_SIZE
#define EXCEPTION_STACK_SIZE    OSZ_CFG_EXCEPTION_STACK_SIZE
#define INTERRUPT_MASK          0x80000000

.section .text.kernel.init
.globl  trap_entry
.globl  exception_handler
.globl  interrupt_handler

.balign 4
trap_entry:
    addi    sp, sp, -16 * REGBYTES
    save_caller_reg
    addi    sp, sp, -16 * REGBYTES
    save_callee_reg         /* the count of callee register is 12 */
    save_csr_reg            /* the count of csr need to save is 2 */

    csrr    a0, mcause
    mv      a1, t0          /* a1 is mepc */
    mv      a2, t1          /* a2 is mstatus */
    csrw    mscratch, sp
    li      t0, INTERRUPT_MASK
    and     t0, a0, t0
    bne     t0, zero, __interrupt_entry
__exception_entry:
    la      sp, EXCEPTION_SP_TOP
    call    exception_handler
    csrw    mepc, a0
    j       __ret_task
__interrupt_entry:
    la      sp, INTERRUPT_SP_TOP
    call    os_preemp_set_int_active
    call    interrupt_handler       /* a0: mepc, a1: mstatus */
    call    is_need_preemp
    beqz    a0, __ret_task
__ret_new_task:
    call os_preemp_clr_int_active
    csrr sp, mscratch
    restore_csr_reg
    restore_callee_reg
    addi sp, sp, 16 * REGBYTES
    restore_caller_reg
    addi sp, sp, 16 * REGBYTES
    call os_preemp_schedule_with_pm
    j    schedule
    
__ret_task:
    call os_preemp_clr_int_active
    csrr sp, mscratch
    restore_csr_reg
    restore_callee_reg
    addi sp, sp, 16 * REGBYTES
    restore_caller_reg
    addi sp, sp, 16 * REGBYTES
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