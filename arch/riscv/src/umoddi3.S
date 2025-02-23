.section .text
.global __umoddi3

# 函数：__umoddi3
# 作用：计算无符号64位整数的模运算 (a % b)
# 参数：a低32位(a0), a高32位(a1), b低32位(a2), b高32位(a3)
# 返回：余数低32位(a0), 余数高32位(a1)
__umoddi3:
    # 保存需要保留的寄存器
    addi sp, sp, -16
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)

    # 保存参数到保存寄存器
    mv s0, a0       # 被除数低32位
    mv s1, a1       # 被除数高32位
    mv s2, a2       # 除数低32位
    mv s3, a3       # 除数高32位

    # 初始化余数为0
    li t1, 0        # 余数低32位
    li t2, 0        # 余数高32位

    # 设置循环计数器为64
    li t0, 64

loop:
    beqz t0, end_loop   # 循环结束条件
    addi t0, t0, -1     # 计数器减1

    # 将被除数左移1位，获取最高位到t3（carry_out）
    srli t3, s1, 31     # 取被除数高32位的最高位
    slli t4, s1, 1      # 高32位左移1位
    srli t5, s0, 31     # 取被除数低32位的最高位
    or t4, t4, t5       # 将低32位的最高位添加到高32位
    slli s0, s0, 1      # 低32位左移1位
    mv s1, t4           # 更新被除数高32位

    # 将余数左移1位，并添加carry_out到最低位
    slli t4, t2, 1      # 余数高32位左移1位
    srli t5, t1, 31     # 取余数低32位的最高位
    or t4, t4, t5       # 将低32位的最高位添加到高32位
    slli t1, t1, 1      # 余数低32位左移1位
    or t1, t1, t3       # 添加carry_out到余数低32位
    mv t2, t4           # 更新余数高32位

    # 比较余数和除数，决定是否相减
    bltu t2, s3, no_subtract    # 比较高32位
    bne t2, s3, subtract        # 高32位不等于
    bltu t1, s2, no_subtract    # 高32位相等，比较低32位

subtract:
    # 执行减法：余数 -= 除数
    mv t5, t1           # 保存原余数低32位
    sub t1, t5, s2      # 低32位相减
    sltu t4, t5, s2     # 检查借位
    sub t6, t2, s3      # 高32位相减
    sub t2, t6, t4      # 减去借位

no_subtract:
    j loop               # 继续循环

end_loop:
    # 将结果移动到返回寄存器
    mv a0, t1
    mv a1, t2

    # 恢复保存的寄存器
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    addi sp, sp, 16

    ret