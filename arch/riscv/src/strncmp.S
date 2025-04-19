.global strncmp
.type strncmp, @function
.section .text

# strncmp 函数：比较两个字符串的前 n 个字符
# 输入：a0 - 字符串1起始地址，a1 - 字符串2起始地址，a2 - 最大比较长度 n
# 输出：a0 - 比较结果（0 表示相等，非零表示字符差值）
strncmp:
    beqz  a2, return_zero    # 若 n=0，直接返回 0
    mv    t3, a2             # 保存 n 到临时寄存器 t3
loop:
    lbu   t0, 0(a0)          # 无符号加载字符串1的当前字符
    lbu   t1, 0(a1)          # 无符号加载字符串2的当前字符
    sub   t2, t0, t1         # 计算字符差值
    bnez  t2, return_diff    # 字符不同，跳转返回差值
    beqz  t0, return_zero    # 字符相同且为 0，字符串已结束，返回 0
    addi  t3, t3, -1         # 剩余比较次数减 1
    beqz  t3, return_zero    # 已比较完 n 个字符且全相同，返回 0
    addi  a0, a0, 1          # 移动字符串1指针
    addi  a1, a1, 1          # 移动字符串2指针
    j     loop               # 继续循环

return_diff:
    mv    a0, t2             # 将字符差值存入返回值 a0
    ret                      # 返回

return_zero:
    li    a0, 0              # 返回 0 表示相等
    ret