# OSZ

取名为**Z**的操作系统，没有什么特殊的含义。

## 当前支持特性

* 中断框架支持
* mtime支持
* ns16550 uart驱动支持
* 多任务支持
* 内存管理支持
* 有且仅有一个外部依赖`printf`（已源码集成）
* qemu-riscv Virt单板支持
* ipc支持（当前仅支持event）
* 抢占调度支持
* shell命令行支持（支持自定义命令）
* - shell 支持上、下、左、右键
* - shell 支持tab补全/联想命令
* - shell 支持任意位置del字符

## 交叉编译工具链信息

代码仓目录下的 `tools/riscv-unknown-elf.tar.gz` 存放了可以直接用于 **riscv32** 编译的交叉编译工具链，工具链源码地址见[这里](https://github.com/riscv-collab/riscv-gnu-toolchain),commit id为`935b263c8ef7f250819c74aeb7736c87ad87ef2b`,编译参数如下：

```shell
./configure --prefix=/home/zyzs/code/riscv_tool_chain/install --with-arch=rv32gc --with-abi=ilp32d --enable-default-pie --with-languages=c,c++ --with-cmodel=medany
```

## 编译及运行

clone该仓库，将`riscv-unknown-elf.tar.gz`压缩包解压到tools目录，然后执行下述命令即可编译运行：

```shell
cd build
./build.sh run

# 调试命令
./build.sh debug
```

## 已集成的第三方库

1. 集成了[嵌入式中最好的printf](https://github.com/mpaland/printf)，实现文件直接拷贝到`kernel/debug`目录下（只有`printf.c`和`printf.h`）;

## 编译框架

### 介绍

编译框架当前为分为三个组件，入口、公共部分及模块私有。
编译入口为`build/Makefile`，通过该文件中的`$(TARGET)`标签遍历`$(MODULES)`进行各模块的编译；公共部分为`build/make/comm.mk`以及`build/make/module.mk`，前者提供全局的路径变量的定义以及公用的一些Makefile宏，后者提供各模块编译依赖的公共部分；模块私有的部分需要在模块的顶层目录下新建Makefile文件，增加并按需修改如下模版即可：
```Makefile
include $(COMM_MK)

MODULE_NAME := 
MODULE := 

C_SRCS = $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)
```

### 增加一个源码组件

当增加的模块提供了源码，且需要编译源码时，定义为新增加一个源码组件，需要按照如下规则执行:

1. 确认`MODULE_NAME`,可以用户自定义，但需要确保与`build/config/osz.config`中的配置`OSZ_CFG_XXX`中的XXX一致;
2. 配置`MODULE`变量为**lib$(MODULE_NAME).a**;
3. 确保待编译的文件已放置在`C_SRCS`变量中;
4. 配置`LOCAL_FLAGS`,新增源文件编译依赖的头文件目录、宏定义、编译选项;
5. 在入口Makefile文件中的`MODULES`变量中增加模块（模块顶层目录的相对路径）;
6. 在`build/config/osz.confg`文件中添加模块配置，即`OSZ_CFG_XXX=y`。

### 增加一个库组件

当新增加的模块是一组不需要编译源文件（已经提供了.a）的静态库时，定义为新增加一个库组件，需要按照如下规则执行:

1. `C_SRCS`、`LOCAL_FLAGS`留空，且`MODULE`变量应该与`MODULE_NAME`变量相等。
2. 组件的目录结构应该为: 
    * `G_OPENSOURCE_PATH`(由build/comm.mk定义)/<组件名> 定义组件顶层目录；
    * `G_OPENSOURCE_PATH`(由build/comm.mk定义)/<组件名> 下应该包含`include`、`lib`、`src`三个目录，如果没有源码，则不需要`src`；
3. 参考 **增加一个源码组件** 一节中的5、6两步将组件添加到编译框架。

## FAQ

1. 当使用`-g`选项生成带调试信息的可执行文件时，链接脚本中不能对`.debug_*`这类段进行指定，应该保持默认；

## 参考信息

* [Makefile 常用函数列表](https://github.com/marmotedu/geekbang-go/blob/master/makefile/Makefile%E5%B8%B8%E7%94%A8%E5%87%BD%E6%95%B0%E5%88%97%E8%A1%A8.md)
* [从头写一个RISCV OS](https://github.com/plctlab/riscv-operating-system-mooc)

## 内核特性

* [启动流程](https://github.com/chenyuxiangg/osz/blob/main/doc/kernel_doc/startup.md)