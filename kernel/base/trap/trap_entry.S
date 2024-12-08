.section .text.kernel.init
.globl trap_entry

.macro  save_reg reg
    sw      x1, 0(\reg)
    sw      x2, 4(\reg)
    sw      x3, 8(\reg)
    sw      x4, 12(\reg)
    sw      x5, 16(\reg)
    sw      x6, 20(\reg)
    sw      x7, 24(\reg)
    sw      x8, 28(\reg)
    sw      x9, 32(\reg)
    sw      x10, 36(\reg)
    sw      x11, 40(\reg)
    sw      x12, 44(\reg)
    sw      x13, 48(\reg)
    sw      x14, 52(\reg)
    sw      x15, 56(\reg)
    sw      x16, 60(\reg)
    sw      x17, 64(\reg)
    sw      x18, 68(\reg)
    sw      x19, 72(\reg)
    sw      x20, 76(\reg)
    sw      x21, 80(\reg)
    sw      x22, 84(\reg)
    sw      x23, 88(\reg)
    sw      x24, 92(\reg)
    sw      x25, 96(\reg)
    sw      x26, 100(\reg)
    sw      x27, 104(\reg)
    sw      x28, 108(\reg)
    sw      x29, 112(\reg)
    sw      x30, 116(\reg)
.endm

.macro  restore_reg reg
    lw      x1, 0(\reg)
    lw      x2, 4(\reg)
    lw      x3, 8(\reg)
    lw      x4, 12(\reg)
    lw      x5, 16(\reg)
    lw      x6, 20(\reg)
    lw      x7, 24(\reg)
    lw      x8, 28(\reg)
    lw      x9, 32(\reg)
    lw      x10, 36(\reg)
    lw      x11, 40(\reg)
    lw      x12, 44(\reg)
    lw      x13, 48(\reg)
    lw      x14, 52(\reg)
    lw      x15, 56(\reg)
    lw      x16, 60(\reg)
    lw      x17, 64(\reg)
    lw      x18, 68(\reg)
    lw      x19, 72(\reg)
    lw      x20, 76(\reg)
    lw      x21, 80(\reg)
    lw      x22, 84(\reg)
    lw      x23, 88(\reg)
    lw      x24, 92(\reg)
    lw      x25, 96(\reg)
    lw      x26, 100(\reg)
    lw      x27, 104(\reg)
    lw      x28, 108(\reg)
    lw      x29, 112(\reg)
    lw      x30, 116(\reg)
    lw      x31, 120(\reg)
.endm

.balign 4
trap_entry:
    csrrw t6, mscratch, t6
    save_reg t6
    csrr    t5, mscratch
    sw      t5, 120(t6)
    csrw    mscratch, t6

    csrr    a0, mcause
    csrr    a1, mepc
    call    trap_handler
    csrw    mepc, a0

    csrr    t6, mscratch
    restore_reg t6
    mret