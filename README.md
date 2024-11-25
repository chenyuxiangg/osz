# ZOS

## 交叉编译工具链信息

代码仓目录下的 `tools/riscv-unknown-elf.tar.gz` 存放了可以直接用于 **riscv32** 编译的交叉编译工具链，工具链源码地址见[这里](https://github.com/riscv-collab/riscv-gnu-toolchain),commit id为`935b263c8ef7f250819c74aeb7736c87ad87ef2b`,编译参数如下：

```shell
./configure --prefix=/home/zyzs/code/riscv_tool_chain/install --with-arch=rv32gc --with-abi=ilp32d --enable-default-pie --with-languages=c,c++ --with-cmodel=medany
```

## 已集成的第三方库

1. 集成了[嵌入式中最好的printf](https://github.com/mpaland/printf)，实现文件直接拷贝到`kernel/debug`目录下（只有`printf.c`和`printf.h`）;

## FAQ

1. 当使用`-g`选项生成带调试信息的可执行文件时，链接脚本中不能对`.debug_*`这类段进行指定，应该保持默认；

## 参考信息

* [Makefile 常用函数列表](https://github.com/marmotedu/geekbang-go/blob/master/makefile/Makefile%E5%B8%B8%E7%94%A8%E5%87%BD%E6%95%B0%E5%88%97%E8%A1%A8.md)
* [从头写一个RISCV OS](https://github.com/plctlab/riscv-operating-system-mooc)