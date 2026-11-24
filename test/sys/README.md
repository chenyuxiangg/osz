# test/sys —— OSZ 板上系统级测试框架

跑在**真实内核上下文**（`app_main` 任务、真中断、真调度、真内存池）里的测试，
不是宿主侧的单元测试。宿主只负责「把镜像跑起来 + 收集日志 + 判定退出码」。

设计文档：`doc/test_doc/板上系统级测试框架设计.html`

## 两条命令

```bash
# 1) 构建测试镜像（独立 build-test/ 目录，产品 build/ 不受影响）
./construct/build_cmake.sh systest                       # -> target_qemu_test
./construct/build_cmake.sh systest target_qemu_shelltest # 产品形态镜像 + test 命令

# 2) 跑套件并出报告（默认本机 wasm 仿真平台后端）
python3 test/sys/runner/run_systest.py \
    --backend sim --elf build-test/targets/qemu_virt/test/target_qemu_test
```

产物在 `build-test/systest-out/`：`console.log` / `result.json` / `junit.xml`。
退出码：`0` 全通过，`1` 有 FAIL/CRASH（或 SKIP，加 `--allow-skip` 可放宽），
`2` 跑不完（早期 trap / 超时 / ELF 不存在 / 后端缺失）。

`--backend qemu` 走 `qemu-system-riscv32 -machine virt -bios none -kernel <elf>`，
需要仓库 `tools/qemu` 已解压或本机已装 qemu；当前机器上只有 sim 后端可用。

## 三个镜像的分工

| 目标 | 自动跑套件 | 有 shell | 用途 |
| --- | --- | --- | --- |
| `target_qemu`（产品） | 否 | 是 | demo 镜像，源码一行不动 |
| `target_qemu_test` | 是（autorun + 哨兵） | 否 | 回归主线，给 runner 用 |
| `target_qemu_shelltest` | 否 | 是 | 产品形态 + `test` 命令，现场定位 |

手动单跑（串口里）：

```
OSZ$ test                          # 列出全部用例
OSZ$ test task.current_identity    # 只跑一条
```

## 目录

```
test/sys/
├── harness/
│   ├── include/osz_test.h     # 用例宏 / 断言 / 恢复 / 报告 API
│   ├── src/assert.c           # 断言：致命 -> longjmp，非致命 -> 记账继续
│   ├── src/report.c           # [SZTEST] v1 行协议
│   ├── src/runner.c           # 状态机、单条执行、名字匹配、时间
│   ├── src/cmd.c              # 产品形态镜像里的 `test` 命令
│   └── arch/setjmp.S          # rv32 最小 setjmp/longjmp（14 个寄存器槽）
├── cases/kernel/<域>/…        # 用例，文件即用例（一文件一条）
└── runner/run_systest.py      # 宿主 runner
```

用例靠 `.osz_test` 段落注册，链接脚本用 `__test_start`/`__test_end` 圈住并
`KEEP`，所以**用例源码必须直接编进可执行文件**，不能做成静态库
（`--gc-sections` 会把没人引用的成员整段丢掉）。

## 加一条用例

```c
#include "osz_test.h"

OSZ_TEST(mem, alloc_free_roundtrip, .tags = TC_TAG_FAST | TC_TAG_MEM, .cost = 2)
{
    TC_ASSERT(p != NULL);
    TC_EXPECT_EQ(ret, (uintptr_t)OS_OK);
}
```

- `OSZ_TEST(域, 名字)` → 报告里的全名是 `域.名字`，也是 `test` 命令的参数。
- 断言：`TC_ASSERT(expr)` / `TC_EXPECT(expr)` / `TC_ASSERT_EQ(expect, actual)` /
  `TC_EXPECT_EQ(expect, actual)` / `TC_SKIP()`。
  致命断言直接终止当前用例（longjmp 回 runner），非致命继续跑并把失败计入用例。
- 新增 `.c` 文件即可，`test/sys/CMakeLists.txt` 用 `GLOB_RECURSE` 收集，
  无需改构建。

## 行协议（`[SZTEST] v1`）

板上只 print 这些行，宿主只解析这些行，两边不共享头文件：

```
[SZTEST] v1 suite_start total=7 board=qemu-virt
[SZTEST] case_begin mem.alloc_free_roundtrip
[SZTEST] assert_fail /abs/path.c:39 "value mismatch"
[SZTEST] assert_detail /abs/path.c:39 expect=0x1 actual=0x80
[SZTEST] case_skip foo.bar "not supported on this board"
[SZTEST] case_end mem.alloc_free_roundtrip FAIL ms=0 asserts=7
[SZTEST] suite_end pass=5 fail=2 crash=0 skip=0 ms=2
[SZTEST] end fails=2
[SZTEST] sentinel OK|FAILED
```

一定要有 `sentinel` 才算跑完——没有它说明镜像早期挂了/超时了，runner 直接判 2。

## 已知边界

1. **本机 wasm 仿真后端不传递异常**：一旦取异常就停机（`halted reason=Trap`），
   所以 `CRASH` 路径（断言外的野指针、非法访问）在 sim 上无法验证，需要在
   QEMU 或真实板上确认。功能用例的 FAIL/PASS 在 sim 上是准的。
2. **时间戳**：`tc_now_ms` 在 sim 上偶发读到异常 mtime（一次读数直接跳到
   2051 秒），harness 里做了单调钳位（倒流或单次前进 >60s 就沿用上次值），
   这只是防御，不是修复；平台侧 mtime 的 64 位读需要单独查。
3. **Kconfig 未接入**：当前用 CMake `option(OSZ_TEST_BUILD)` + 全局
   `-DOSZ_CFG_TEST=1`，`kconfiglib` 装上以后再切到 Kconfig 开关。
4. 产品 `target_qemu` 若要和测试镜像同处一个构建目录，`exception_handler`
   里的恢复钩子是**弱符号**，没链 harness 时自动退回原来的打印+停机行为。

## 顺带修掉的内核问题

`kernel/startup/start.S` 与 `kernel/base/trap/src/trap_entry.S` 里的启动栈/
中断栈原来用 `.section .stack`（PROGBITS），链接器把它排在堆之前，导致
`__system_heap_start` 与启动栈**重叠**——内存池初始化直接踩掉启动栈。
测试镜像因此上电即 trap。现已改成 `.section .stack, "aw", @nobits`，
栈不再进镜像、堆从栈之后开始（`__system_heap_start = 0x84006920`）。
