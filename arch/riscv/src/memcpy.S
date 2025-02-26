.global memcpy
.type memcpy, @function
.section .text

# memcpy: 将n字节从src复制到dest
# a0: dest指针 (返回值为dest)
# a1: src指针
# a2: 字节数n
memcpy:
    mv      t0, a0             # 保存原始dest地址用于返回

    # 检查dest和src地址偏移是否相同
    andi    t1, a0, 0x3        # 获取dest地址的低2位
    andi    t2, a1, 0x3        # 获取src地址的低2位
    bne     t1, t2, byte_copy  # 偏移不同则直接按字节复制

align_loop:                     # 对齐处理循环
    beqz    t1, aligned         # 已对齐则跳转到字复制
    beqz    a2, end             # n=0时提前返回
    lb      t3, 0(a1)          # 加载1字节
    sb      t3, 0(a0)          # 存储1字节
    addi    a0, a0, 1          # dest++
    addi    a1, a1, 1          # src++
    addi    a2, a2, -1         # n--
    addi    t1, t1, -1         # offset--
    andi    t1, t1, 0x3        # 保持offset在0-3范围
    j       align_loop

aligned:                        # 对齐后的字复制部分
    srli    t3, a2, 2          # t3 = 需要复制的字数(n/4)
    beqz    t3, byte_remain     # 没有完整字则跳转到字节处理

word_loop:
    lw      t4, 0(a1)          # 加载1个字(4字节)
    sw      t4, 0(a0)          # 存储1个字
    addi    a1, a1, 4          # src +=4
    addi    a0, a0, 4          # dest +=4
    addi    t3, t3, -1         # 字数计数器--
    bnez    t3, word_loop      # 继续循环直到所有字复制完成

byte_remain:                    # 处理剩余0-3字节
    andi    a2, a2, 0x3        # 获取剩余字节数(n%4)
    beqz    a2, end             # 无剩余字节则结束

remain_loop:
    lb      t5, 0(a1)          # 加载1字节
    sb      t5, 0(a0)          # 存储1字节
    addi    a1, a1, 1          # src++
    addi    a0, a0, 1          # dest++
    addi    a2, a2, -1         # 剩余字节数--
    bnez    a2, remain_loop    # 继续直到所有字节复制完成
    j       end

byte_copy:                      # 完全按字节复制的路径
    beqz    a2, end             # 处理n=0的特殊情况

byte_loop:
    lb      t6, 0(a1)          # 加载1字节
    sb      t6, 0(a0)          # 存储1字节
    addi    a1, a1, 1          # src++
    addi    a0, a0, 1          # dest++
    addi    a2, a2, -1         # n--
    bnez    a2, byte_loop      # 继续直到所有字节复制完成

end:
    mv      a0, t0             # 返回原始dest地址
    ret                         # 函数返回