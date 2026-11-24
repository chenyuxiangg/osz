## 交叉编译链工具配置方法

### 一、CMake 构建（`construct/build_cmake.sh`，推荐）

`construct/cmake/riscv32_toolchain.cmake` 已改为自适应入口，**不写死前缀、目录结构和 gcc 版本号**，按以下顺序查找工具链：

1. `cmake -D` 参数：`-DOSZ_CROSS_ROOT=<工具链根目录> -DOSZ_CROSS_PREFIX=<前缀>`
2. 同名环境变量：`OSZ_CROSS_ROOT` / `OSZ_CROSS_PREFIX`
3. 仓库内 `tools/toolchain`（**不纳入 git**，可放解压目录或符号链接，支持 `<root>/bin` 与 `<root>/<prefix>/bin` 两种布局）
4. 系统 `PATH`

换一台机器最少只需两个环境变量：

```shell
export OSZ_CROSS_ROOT=/opt/xpack-riscv-none-elf-gcc-15.2.0-1
export OSZ_CROSS_PREFIX=riscv-none-elf      # 官方 riscv-gnu-toolchain 填 riscv32-unknown-elf
./construct/build_cmake.sh target_qemu
```

可选参数：

| 变量 | 默认 | 说明 |
|---|---|---|
| `OSZ_CROSS_ARCH` | `rv32g` | 目标架构 |
| `OSZ_CROSS_ABI` | `ilp32d` | 目标 ABI |
| `OSZ_REQUIRE_CXX` | `OFF` | `TRUE` = 工具链必须带 C++ 前端（`cc1plus`），缺则配置阶段直接失败 |

实现要点：

1. 前缀自动探测：用户给定 → `riscv32-unknown-elf` → `riscv-none-elf` → `riscv64-unknown-elf`（配 `-march=rv32*`）。
2. `sysroot` 与 `libgcc` 路径由编译器自己回答（`--print-sysroot`、`-print-file-name=libgcc.a`），升级 gcc 版本无需改脚本。
3. C++ 语言按需启用：内核无 C++ 翻译单元，精简工具链（缺 `cc1plus`）会自动跳过 `CXX`；CI 想强制完整工具链时设 `-DOSZ_REQUIRE_CXX=TRUE`。
4. 每个工具（`ar`/`objcopy`/…）用独立的 `find_program` 变量名注册，避免 CMake 缓存导致工具错认。

### 二、Makefile 构建（`construct/build.sh`，旧路径）

1. 将工具链压缩包放置在`/home/<user>/tools/toolchains`目录下，比如：`/home/zyzs/tools/toolchains/riscv32-unknown-elf.tar.gz`;
2. 修改*construct/script/env.sh*文件的`compiler_name`变量为交叉工具链名，比如`compiler_name=riscv32-unknown-elf`;

### 三、Makefile 路径的原理

1. `construct/build.sh`运行前会先加载`construct/script/env.sh`;
2. `construct/script/env.sh`会去固定地址读取编译工具链，固定地址默认为`/home/<user>/tools/toolchains`;
3. `construct/script/env.sh`会检查固定地址处是否存在`${compiler_name}.tar.gz`的压缩包，若压缩包名匹配，或者后缀不匹配，则脚本会报错**Error: not fount ${compiler_path}.tar.gz**;
4. 对编译工具链解压缩后，`construct/script/env.sh`会将交叉编译链可执行文件路径（CROSS_COMPILER_PATH）、仿真器路径（QEMU_PATH）、Makefile公共依赖（COMM_MK）、内核配置文件（CFG_MK）、Makefile编译公共流程（MODULE_MK）、输出文件路径（OUTPUT_PATH）导出，用于后续编译。

> 注意：
> 1. Makefile公共依赖（COMM_MK）、Makefile编译公共流程（MODULE_MK）请务必不要修改，除非你知道修改后的明确影响；
> 2. 内核配置文件（CFG_MK）直接修改后不会生效，该文件为`construct/script/parser_cfg.py`解析`construct/config/osz.config`后自动生成。
