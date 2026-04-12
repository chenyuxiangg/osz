# 项目运行验证证指南

## 概述

该指南适用于验证项目运行是否正确，任何修改/删除/新增**.c**、**.h**、**CMakeLists.txt**以及**Makefile**的操作都必须验证运行的正确性。

## 核心流程

### 检查环境变量

如果`ENV_OSZ_ROOT`为空，则提示用户需要配置该环境变量为OSZ项目的根目录，即**construct**所在目录，然后上报构建错误及原因-未指定项目根目录。
否则可进行下一步。

### 运行qemu virt测试

执行命令：

```shell
cd $ENV_OSZ_ROOT && ./construct/build_cmake.sh target_qemu_run
```

### 校验运行结果

有如下输出则表明测试通过：

```txt
==== Enter Main ====
OSZ$
```

### 关闭qemu

通过按键ctrl + A + X退出qemu或直接通过kill命令结束qemu进程。