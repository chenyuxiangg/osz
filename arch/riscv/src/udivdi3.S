.section .text
.global __udivdi3

# 函数：__udivdi3
# 作用：计算无符号64位整数的除法运算 (a / b)
# 参数：a低32位(a0), a高32位(a1), b低32位(a2), b高32位(a3)
# 返回：商低32位(a0), 商高32位(a1)
__udivdi3:
    # 保存需要保留的寄存器
    addi sp, sp, -16
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)

    # 保存参数到保留寄存器
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

    # 取出被除数的高位最高位（s1的bit31）
    srli t3, s1, 31     # t3 = 0或1（被除数当前最高位）

    # 余数左移1位并添加被除数的当前最高位到最低位
    slli t4, t2, 1      # 余数高32位左移
    srli t5, t1, 31     # 取余数低32位的最高位
    or t4, t4, t5       # 合并到高32位
    slli t1, t1, 1      # 余数低32位左移
    or t1, t1, t3       # 添加被除数的最高位到余数最低位
    mv t2, t4           # 更新余数高32位

    # 被除数左移1位（逐步构建商）
    srli t4, s0, 31     # 取被除数低32位的最高位
    slli s0, s0, 1      # 低32位左移
    slli s1, s1, 1      # 高32位左移
    or s1, s1, t4       # 将低32位的最高位添加到高32位

    # 比较余数和除数，决定是否相减
    bltu t2, s3, no_subtract    # 余数高位 < 除数高位，不执行减法
    bne t2, s3, subtract        # 余数高位 > 除数高位，执行减法
    bltu t1, s2, no_subtract    # 高位相等时，比较低位

subtract:
    # 执行减法：余数 -= 除数
    mv t5, t1           # 保存原余数低32位
    sub t1, t5, s2      # 低32位相减
    sltu t4, t5, s2     # 检查借位
    sub t6, t2, s3      # 高32位相减
    sub t2, t6, t4      # 减去借位

    # 设置商的最低位为1（当前被除数已左移，最低位为0）
    addi s0, s0, 1      # 直接设置最低位

no_subtract:
    j loop               # 继续循环

end_loop:
    # 将商移动到返回寄存器
    mv a0, s0
    mv a1, s1

    # 恢复保存的寄存器
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    addi sp, sp, 16

    ret