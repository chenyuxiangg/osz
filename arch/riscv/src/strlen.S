.global strlen
.type strlen, @function
.section .text

# strlen 函数：计算以空字符结尾的字符串长度
# 输入：a0 - 字符串起始地址
# 输出：a0 - 字符串长度（不含空字符）
strlen:
    mv    t0, a0       # 保存起始地址到 t0
loop:
    lb    t1, 0(a0)    # 加载当前字符到 t1
    beqz  t1, end      # 若字符为 0，结束循环
    addi  a0, a0, 1    # 指针递增
    j     loop         # 继续循环
end:
    sub   a0, a0, t0   # 计算长度（当前地址 - 起始地址）
    ret                # 返回结果