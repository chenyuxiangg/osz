# 项目构建验证指南

## 概述

该指南适用于验证项目构建工程是否正确，任何修改/删除/新增**.c**、**.h**、**CMakeLists.txt**以及**Makefile**的操作都必须验证构建的正确性。

## 核心流程

### 检查环境变量

如果`ENV_OSZ_ROOT`为空，则提示用户需要配置该环境变量为OSZ项目的根目录，即**construct**所在目录，然后上报构建错误及原因-未指定项目根目录。
否则可进行下一步。

### 编译命令

```shell
cd $ENV_OSZ_ROOT && ./construct/build_cmake.sh target_qemu_disasm
```

### 校验编译结果

#### 校验输出

如果编译输出有**Build completed successfully.**字样，则编译成功；否则编译失败，并上报具体的错误原因。

#### 校验二进制文件

执行命令：

```shell
find $ENV_OSZ_ROOT -name target_qemu
```

如果存在**target_qemu**则检查二进制成功，否则检查失败，并上报错误原因-无法找到二进制文件。