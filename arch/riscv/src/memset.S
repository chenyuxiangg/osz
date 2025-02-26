.global memset
.type memset, @function
.section .text

# memset: 用指定字节值填充内存区域
# a0: 目标地址dest (返回值为dest)
# a1: 填充字节值（低8位有效）
# a2: 字节数n
memset:
    mv      t0, a0             # 保存原始dest地址用于返回
    beqz    a2, end            # 处理n=0的特殊情况

    # 将字节值扩展到32位字
    andi    t1, a1, 0xff       # 确保只使用低8位
    slli    t2, t1, 8          # 第2字节位置
    or      t1, t1, t2
    slli    t2, t1, 16         # 扩展到高16位
    or      t1, t1, t2         # t1 = 0xBBBBBBBB（BB为输入字节）

    # 对齐处理（对齐到4字节边界）
    andi    t2, a0, 0x3        # 获取地址低2位
    beqz    t2, word_fill      # 已对齐则直接进入字填充

align_loop:                     # 对齐目标地址
    sb      t1, 0(a0)          # 存储1字节
    addi    a0, a0, 1          # dest++
    addi    a2, a2, -1         # n--
    andi    t2, a0, 0x3        # 检查新地址对齐
    beqz    t2, word_fill      # 对齐后跳转
    bnez    a2, align_loop     # 继续直到对齐或n=0
    j       end

word_fill:                      # 批量字填充
    srli    t3, a2, 2          # 计算完整字数（n/4）
    beqz    t3, byte_remain    # 没有完整字则跳过

word_loop:
    sw      t1, 0(a0)          # 存储1个字（4字节）
    addi    a0, a0, 4          # dest +=4
    addi    t3, t3, -1         # 字数计数器--
    bnez    t3, word_loop      # 继续循环

byte_remain:                    # 处理剩余0-3字节
    andi    a2, a2, 0x3        # 获取剩余字节数
    beqz    a2, end            # 无剩余则结束

remain_loop:
    sb      t1, 0(a0)          # 存储1字节
    addi    a0, a0, 1          # dest++
    addi    a2, a2, -1         # 剩余字节数--
    bnez    a2, remain_loop

end:
    mv      a0, t0             # 返回原始dest地址
    ret