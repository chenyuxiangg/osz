.global strncpy
.type strncpy, @function
.section .text

# strncpy 函数：将源字符串的前 n 个字符复制到目标地址
# 输入：a0 - 目标地址 dest，a1 - 源地址 src，a2 - 最大复制长度 n
# 输出：a0 - 返回目标地址 dest（保持原始输入值）
# 行为：
#   1. 复制 src 的前 n 个字符到 dest
#   2. 若 src 长度不足 n，剩余空间用 0 填充
#   3. 若 src 中存在 '\0'，后续字符均置 0
strncpy:
    mv    t0, a0         # 保存原始 dest 地址到 t0（用于返回）
    beqz  a2, end        # 若 n=0，直接跳转到结尾返回

copy_loop:
    lbu   t1, 0(a1)      # 无符号加载 src 当前字节到 t1
    sb    t1, 0(a0)      # 将字节存储到 dest
    addi  a0, a0, 1      # dest 指针递增
    addi  a1, a1, 1      # src 指针递增
    addi  a2, a2, -1     # 剩余复制次数 n-1
    beqz  a2, end        # 若 n=0，结束复制
    bnez  t1, copy_loop  # 若当前字符非 0，继续循环

fill_zero:               # 遇到 '\0' 后的填充阶段
    beqz  a2, end        # 若剩余次数为 0，结束
    sb    zero, 0(a0)    # 向 dest 写入 0
    addi  a0, a0, 1      # dest 指针递增
    addi  a2, a2, -1     # 剩余次数 n-1
    j     fill_zero      # 继续填充

end:
    mv    a0, t0         # 恢复原始 dest 地址到 a0（返回值）
    ret                  # 返回