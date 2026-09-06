# OSZ 单元测试框架（host-side gtest）

本目录是 OSZ 项目的主机端（x86 / Linux）单元测试框架。它独立于 RISC-V
交叉编译工具链，使用宿主机的 `gcc` / `g++` 直接编译选定的 OSZ 源码文
件，并链接 GoogleTest 运行用例。

## 为什么是 host-side？

OSZ 是裸机 RTOS。在 RISC-V 目标上没有标准库、没有文件系统、没有 stdout，
直接运行 gtest 不现实。因此：

- 我们挑选**与架构无关的纯 C 模块**（数据结构 `kernel/struct/*`、通用工具
  `kernel/comm/src/*`），在 host 上以原生方式编译、运行。
- 依赖中断 / 任务 / 堆 / 汇编启动的代码（`kernel/base/*`、`kernel/startup/*`、
  `arch/*` 等）暂不纳入测试范围。

## 目录结构

```
test/
├── CMakeLists.txt              # 独立测试构建入口（FetchContent + gtest）
├── runtest.sh                  # 一键运行脚本
├── README.md                   # 本文件
├── cmake/
│   └── quote_wrapper.sh        # 把 CMake 合并后的 -iquote 展开为逐目录一个
├── include/                    # 编译 OSZ 源码所需的 host 侧 stub 头
│   ├── assert.h                # 把 OSZ 的 ASSERT(x) 改为 fprintf + abort
│   ├── menuconfig.h            # 空 stub（OSZ 自动生成的 menuconfig.h）
│   ├── printf.h                # 把 printf 桥接到 stdio
│   ├── stdarg.h, stdint.h, stdbool.h, float.h, string.h
│   │                           # 转发到系统头，让 `<>` 形式仍命中 glibc
│   ├── ztype.h                 # 本地 typedef + 系统的 size_t / int8_t
│   └── osz_host_preinclude.h   # 顶层强制 include，串起所有 stub
├── stubs/
│   └── assert_stub.c           # 提供 void assert() 的 host 实现
└── unit/
    ├── CMakeLists.txt          # 中间层：定义 add_osz_test、递归发现模块
    └── kernel/                 # ← 镜像 kernel/ 目录结构
        ├── comm/
        │   └── strtoul/
        │       ├── CMakeLists.txt  # 注册 test_strtoul
        │       └── test_strtoul.cpp
        └── struct/
            ├── dlink/
            │   ├── CMakeLists.txt  # 注册 test_dlink
            │   └── test_dlink.cpp
            └── sortlink/
                ├── CMakeLists.txt  # 注册 test_sortlink
                └── test_sortlink.cpp
```

每个测试模块是 `test/unit/kernel/<分类>/<name>/` 下一个独立目录，含
自己的 `CMakeLists.txt` 与 `test_<name>.cpp`。路径从 `unit/` 到模块
目录的叶子名（`<name>`）就是 ctest label（小写，无前缀）；分类层
（`comm` / `struct` 等）只是**镜像** `kernel/` 的同名目录，便于对照
查看，不参与选择。

## 快速开始

```bash
# 一键：配置 + 编译 + 运行（所有模块）
./test/runtest.sh

# 只编译（不跑）
./test/runtest.sh --build

# 只跑（跳过编译）
./test/runtest.sh --run

# 全量重建
./test/runtest.sh --rebuild

# 清理构建目录
./test/runtest.sh --clean
```

脚本等价于：

```bash
cmake -S test -B test/build -DCMAKE_BUILD_TYPE=Debug
cmake --build test/build -j
ctest --test-dir test/build --output-on-failure
```

### 选择**编译哪些**模块（CMake 层）

通过缓存变量 `OSZ_TEST_MODULES` 控制要编译的测试模块。`runtest.sh`
默认会把所有模块编译进去；想节省时间可以用 `-D` 直接传给 cmake：

| 值 | 行为 |
|---|---|
| `all`（默认） | 递归发现 `test/unit/` 下所有含 `CMakeLists.txt` 的叶子目录并全部编译 |
| `none` | 只配置项目，不编译任何测试（gtest / osz_host_stubs 仍正常构建） |
| `dlink;strtoul` | 分号分隔的白名单，仅编译这些叶子目录对应的模块 |

```bash
# 只编译 dlink + strtoul（不编译 sortlink）
cmake -S test -B test/build -DOSZ_TEST_MODULES="dlink;strtoul"

# 只做配置，不编译任何测试
cmake -S test -B test/build -DOSZ_TEST_MODULES="none"

# 模块名拼错时会立刻报错并列出已发现的 label
cmake -S test -B test/build -DOSZ_TEST_MODULES="dlink;oops"
# CMake Error: OSZ_TEST_MODULES: no test module labelled 'oops'
#              (known labels: dlink;sortlink;strtoul)
```

**注意**：这是**编译期**选项。改完需要重新 cmake（`runtest.sh` 自动处理）。
想对已编译好的测试**只跑某几个**仍用下面小节介绍的 `--module` /
`--gtest-filter`，不要动 `OSZ_TEST_MODULES`。

## 按模块 / 按用例选择

测试框架支持三种粒度的选择，从粗到细：

### 1. 整模块选择：`runtest.sh --module`（推荐）

模块名即 ctest label（小写，无 `test_` 前缀）。当前可用模块：
`dlink` / `sortlink` / `strtoul`。

```bash
# 只跑 dlink + strtoul（跳过 sortlink）
./test/runtest.sh --module dlink,strtoul

# 单个模块
./test/runtest.sh --run --module sortlink

# 多次 --module 累加
./test/runtest.sh --module dlink --module strtoul

# 排除某个模块
./test/runtest.sh --module dlink --exclude-module sortlink
```

底层实现：`runtest.sh` 把模块列表转成 `ctest -L <regex>`。详细报告
里仍能看到 Label Time Summary。

### 2. ctest 过滤：`ctest -L` / `-R` / `-LE`

直接调用 ctest，用 label 或名字匹配：

```bash
# 按 label（=模块名）
ctest --test-dir test/build -L dlink

# 按可执行文件名字（regex）
ctest --test-dir test/build -R "test_(dlink|strtoul)"

# 排除（与 -L / -R 可组合）
ctest --test-dir test/build -LE sortlink
ctest --test-dir test/build -R "test_.*" -E "test_sortlink"

# 列出所有 label
ctest --test-dir test/build --print-labels
```

### 3. 细到单个 gtest 用例：`--gtest-filter`

最细粒度。需要把 ctest 旁路，直接调用 gtest 二进制（`runtest.sh`
已经封装好）：

```bash
# 只跑名为 InitProducesEmptySentinel 的用例（跨所有模块）
./test/runtest.sh --run --gtest-filter 'DLink.InitProducesEmptySentinel'

# 限制到 dlink 模块再过滤
./test/runtest.sh --run --module dlink --gtest-filter '*DelNode*'

# dlink + strtoul 中所有 *Overflow* 用例
./test/runtest.sh --run --module dlink,strtoul --gtest-filter '*Overflow*'

# 排除用例：负号前缀
./test/runtest.sh --run --module dlink --gtest-filter '-DLink.Foreach*'
```

gtest filter 语法：`Suite.TestName`，`*` 通配，`-` 前缀表示排除，
用 `:` 分隔多个模式（详见 `test_<x> --gtest_help`）。

### 三种用法的取舍

| 方式 | 粒度 | 适用场景 |
|---|---|---|
| `--module` | 整模块 | CI / 日常回归；想看 ctest 汇总报告 |
| `ctest -L/-R/-LE` | 整模块（regex） | 想跨项目统一调用 ctest；批量脚本 |
| `--gtest-filter` | 单个用例 | 本地调试；只想看一个 case 是否通过 |

## 如何工作

### 1. shadow stub — 让 host 头替换 OSZ 头

OSZ 的 `assert.h` / `stdarg.h` / `stdint.h` / `ztype.h` 等是为裸机编写
的，与 glibc 冲突（类型宽度、`__NEED_*` 宏、`UINTPTR` 在 RV32 上只有
32 位等）。处理方式是：

- `test/include/` 放同名 stub。stub 通过自己的 include guard 在 OSZ
  源码 `comm.h` `#include "stdarg.h"` 之前先被预处理过，从而让 OSZ
  版本的同名头被整体跳过。
- `osz_host_preinclude.h` 是顶层强制 include（`-include` flag），由
  CMake 在每个 TU 编译时首先拉入。它先拉 glibc 的 `<stdarg.h>` /
  `<stdint.h>`，再拉所有 stub，保证 `::int8_t` 等被正确定义。
- 编译选项里所有 OSZ 路径都加为 **`-iquote`**（不是 `-I`），确保
  `#include <...>` 形式**永远**跳过 OSZ 头、命中 glibc。只有
  `#include "..."` 形式才会进入 `-iquote` 路径。

`test/include/ztype.h` 同时定义 OSZ 风格的 `INT8/UINT32/BOOL/...`
typedef，并在 host 上把 `UINTPTR` 拓宽到 `uintptr_t`（避免 C++ 在
x86_64 上对 `(UINTPTR)ptr` 报精度丢失错误）。

### 2. `-iquote` 多路径问题

GCC 的 `-iquote <dir>` 只接受**一个**目录参数。但 CMake 的
Unix Makefiles / Ninja 生成器会把多个 `-iquote` 合并成
`-iquote dir1 dir2 ...`，使 GCC 只看到第一个目录。

`cmake/quote_wrapper.sh` 作为编译器 launcher（通过
`CMAKE_C/CXX_COMPILER_LAUNCHER`）拦截这一调用，把合并后的
`-iquote ...` 重新拆成一对一对的 `-iquote <dir>` 再交给真正的
`gcc` / `g++`。

### 3. 每个模块一个独立 host 静态库 + 自己的 include 路径

顶层 `test/CMakeLists.txt` **不**列出任何 OSZ 源文件，也**不**维护
"所有模块的 include 路径"清单。每个模块的 `CMakeLists.txt` 是自包含
的：

- `add_osz_test(<name> <test.cpp> <osz.c>...)` 把 .c 编进模块专属
  静态库 `osz_host_<label>`，链接到 gtest 可执行文件。
- `OSZ_MODULE_INCLUDE_DIRS`（在该 CMakeLists 里 set）声明本模块 TU
  链所需的 include 目录；`add_osz_test` 内部会把**所有** -iquote 路径
  （common + module）一次性加到编译选项里。

后果：

- 编译跟选择完全对应：`-DOSZ_TEST_MODULES=strtoul` 只会编 `strtoul.c`，
  不会浪费算力编 `dlink.c` / `sortlink.c`。
- 顶层没有"模块清单"或"include 清单"需要跟 `test/unit/` 保持同步
  ——新增模块 = 新建一个目录，零顶层编辑。
- 模块的 include 依赖**就在它自己**的 CMakeLists.txt 里：读它就能
  知道"这个测试需要 `comm/include` 提供的 `offset.h` 等"。底层不被
  任何本模块用到的目录不会出现在编译选项里。

模块间的依赖（如 `sortlink` 调 `dlink_*`）通过把被依赖方的 .c 文件
**也列进**依赖方的 `add_osz_test` 来解决；两者会一起编进同一个静态
库，链接器自然解析符号。这比维护一份"模块依赖图"简单得多，代价仅
是若干 .o 文件被多编一次（静态库内部，链接时取其一即可）。

### 4. 其它

`test/stubs/assert_stub.c` 提供 `void assert()` 的定义，保证任何
仍声明该函数的 OSZ 代码可以链接。

## 当前覆盖

| 模块            | 文件                                                  | 测试要点 |
|-----------------|-------------------------------------------------------|----------|
| `dlink`         | `unit/kernel/struct/dlink/test_dlink.cpp`             | init / empty / insert tail & head / 删除节点 / 迭代顺序 |
| `sortlink`      | `unit/kernel/struct/sortlink/test_sortlink.cpp`       | init / 单条插入 / 降序与升序插入的 delta 行为 / 零超时拒绝 / 删除节点合拢 gap |
| `strtoul`       | `unit/kernel/comm/strtoul/test_strtoul.cpp`           | 自动检测进制（0/8/16） / 显式进制 / 空白与符号 / 负号取反 / 溢出饱和 / `endptr` 行为 |

## 添加新测试

1. 在 `test/unit/kernel/<分类>/` 下新建 `<name>/` 子目录（`<分类>` 与
   被测模块在 `kernel/` 下的路径对应，例如 `kernel/struct/dlink.c`
   → `test/unit/kernel/struct/dlink/`）。
2. 在 `<name>/test_<name>.cpp` 里按 gtest 规范编写用例；OSZ 头文件
   请通过 `extern "C" { #include "xxx.h" }` 引入。
3. 在 `<name>/CMakeLists.txt` 写：
   ```cmake
   # 本模块 TU 链所需的 include 目录（-iquote）。
   # test/include 下的 stub 由框架自动加，无需列在这里。
   set(OSZ_MODULE_INCLUDE_DIRS
       "${OSZ_KERNEL_DIR}/comm/include"            # 或者别的模块目录
       # ...
   )

   # 被测 OSZ 源文件 + 间接依赖（见下）。
   add_osz_test(test_<name>
       test_<name>.cpp
       "${OSZ_<DIR>}/<path>/<file>.c"   # 被测 OSZ 源文件
       # ... 可能的间接依赖
   )
   ```
   下次 `runtest.sh` 会自动递归发现这个子目录（叶子名按字典序），
   并把它加进 ctest（label = `<name>`）。`add_osz_test` 内部会把所
   列 OSZ 源文件编进一个独立的静态库 `osz_host_<name>`，再链接到
   gtest 可执行文件——所以 **不需要** 改顶层 [test/CMakeLists.txt](CMakeLists.txt)。

   `OSZ_MODULE_INCLUDE_DIRS` 这一变量告诉框架"本模块需要哪些
   include 目录"：你的 .cpp / .c 文件 `#include "xxx.h"` 时哪些目
   录要被搜索。top-level 不会替你维护这个列表。

4. **间接依赖**：如果被测的 OSZ .c 文件调用了别的 OSZ 模块的函数
   （比如 `sortlink.c` 调用 `dlink_init / dlink_insert_*`），把那
   些 .c 文件**也**加到 `add_osz_test` 的参数里。它们会和被测文件
   一起编进 `osz_host_<name>`，链接器自然能解析所有符号。同时别忘
   了把被依赖模块的 include 目录加进 `OSZ_MODULE_INCLUDE_DIRS`（例
   如 sortlink 调 dlink 需要 `double_link/include`）。
5. 如果新模块依赖了目前没有 stub 的 OSZ 头（例如 `string.h`、`mem.h`），
   在 `test/include/` 添加对应的 stub。
6. 只想编这个新模块先试一下：`-DOSZ_TEST_MODULES=<name>`。

## 与主构建系统的关系

- 本测试构建**与 RISC-V 交叉构建完全隔离**——它使用宿主编译器，构建产物
  输出到 `test/build/bin/`，不会进入 `output/` 或 `tools/toolchain/`
  任何路径。
- `construct/cmake/comm.cmake` 中已注释掉的 `add_gtest_executable` 骨架
  保留不动——那是给将来主构建内嵌测试预留的位置，与本框架互不影响。

## 故障排查

- **`gtest` 拉取失败**：检查网络能否访问 `github.com`；或手动下载
  `v1.14.0.tar.gz` 放到本地后再重试。
- **`ASSERT failed` 出现在 gtest 报告里**：OSZ 源码被调用了 NULL 指针参数。
  这是真实的源码 bug，应当通过测试用例暴露出来。
- **找不到 `assert()` 符号**：确保 `osz_host_stubs` 被链接进测试可执行
  文件；CMake 中已配置好，若自行加 target 请记得 link。
- **`cc: warning: ... linker input file unused`**：通常说明
  `cmake/quote_wrapper.sh` 没生效；检查 `CMAKE_C_COMPILER_LAUNCHER`
  是否被覆盖、脚本是否有可执行权限。
- **奇怪的 `int8_t has not been declared in ::`**：preinclude 没跑；
  检查 `-include test/include/osz_host_preinclude.h` 是否仍在编译命令
  里（`test/build/.../flags.make`）。