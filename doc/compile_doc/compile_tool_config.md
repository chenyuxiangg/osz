## 交叉编译链工具配置方法

1. 将工具链压缩包放置在`/home/<user>/tools/toolchains`目录下，比如：`/home/zyzs/tools/toolchains/riscv32-unknown-elf.tar.gz`;
2. 修改*construct/script/env.sh*文件的`compiler_name`变量为交叉工具链名，比如`compiler_name=riscv32-unknown-elf`;

## 原理

1. `construct/build.sh`运行前会先加载`construct/script/env.sh`;
2. `construct/script/env.sh`会去固定地址读取编译工具链，固定地址默认为`/home/<user>/tools/toolchains`;
3. `construct/script/env.sh`会检查固定地址处是否存在`${compiler_name}.tar.gz`的压缩包，若压缩包名匹配，或者后缀不匹配，则脚本会报错**Error: not fount ${compiler_path}.tar.gz**;
4. 对编译工具链解压缩后，`construct/script/env.sh`会将交叉编译链可执行文件路径（CROSS_COMPILER_PATH）、仿真器路径（QEMU_PATH）、Makefile公共依赖（COMM_MK）、内核配置文件（CFG_MK）、Makefile编译公共流程（MODULE_MK）、输出文件路径（OUTPUT_PATH）导出，用于后续编译。

> 注意：
> 1. Makefile公共依赖（COMM_MK）、Makefile编译公共流程（MODULE_MK）请务必不要修改，除非你知道修改后的明确影响；
> 2. 内核配置文件（CFG_MK）直接修改后不会生效，该文件为`construct/script/parser_cfg.py`解析`construct/config/osz.config`后自动生成。