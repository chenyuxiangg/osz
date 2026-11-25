# 仿真平台需求单：陷阱交付后不应停止 + mtime 64 位读一致性

- 提出方：OSZ（板上系统级测试框架 P0，仓库 `chenyuxiangg/osz`）
- 平台：`riscv_sim`（wasm 内核 + 本机 `run_on_platform.mjs`），板模板 `qemu-virt`
- 日期：2026-11-25
- 优先级：R-1 = P0（阻塞 OS 级测试的可信度）、R-2 = P1

---

## R-1　陷阱已交付，但后端把"发生陷阱"当成了停止条件

### 现象

用 `qemu-virt` 模板跑一份正常的 RISC-V RTOS 镜像（OSZ），执行到一条非法访存
（用例里故意写野指针）时：

```
[sim] halted: {"reason":"Trap","pc":0x8000d5d0,"trigger_addr":null}
```

`StepN` 只跑了几十万条就停，console 停在错误之前，宿主拿不到"guest 自己的
exception handler 处理完之后的结果"。同一份 ELF 在 `qemu-system-riscv32 -machine virt`
上，guest 的 `mtvec` handler 正常执行、异常被 guest 消费，后续用例继续跑。

### 平台侧定位（供确认，不是猜测）

1. ISS 在指令执行时就完成了**陷阱交付**：
   `crates/kernel/src/m13_iss/executor.rs:894 / 912 / 1088 / 1102 / 1130` 调用
   `csr::enter_trap(...)`（`crates/kernel/src/m13_iss/csr.rs:667`），即写
   `mepc/mcause/mtval`、按 `medeleg` 判委托、跳 `mtvec`、更新 priv。
2. 交付完成后，M-17 后端把它当成**停止**：
   `crates/kernel/src/kernel_host/execute.rs` 与
   `crates/kernel/src/m17_backend/mod.rs:713`：

   ```rust
   Ok(StepOutcome::Trapped { .. }) => return Ok(ExecOutcome::Stop {
       reason: HaltReason::Trap, consumed: true,
   }),
   ```

也就是说：**问题不是"仿真器不会处理异常"，而是"处理完立刻停机"**。
`HaltReason::Trap` 的注释仍写着 "Illegal instruction or bus error"，
但实际语义已经等于"这条指令触发了一个陷阱"。

### 影响

1. **OS/RTOS 场景无法验证异常路径**：guest 的 `exception_handler` 永远拿不到控制权，
   野指针、非法指令、`ebreak`、甚至是内核自己的异常测试用例，全都表现为"仿真停止"。
2. **与真机行为不一致**：真板/QEMU 上这类异常由 guest 消费，仿真器上却像崩溃，
   容易把"guest 正常处理了一个异常"误判成 OS bug 或平台 bug。
3. **具体阻塞项**：OSZ 板上测试框架的 `CRASH` 用例（断言外野指针 → 记 CRASH →
   后续用例继续）、异常可恢复验证、P2 的"用 mtime 中断改写被中断任务上下文做硬超时"，
   全卡在这一条上。

### 需求

新增一个**停止策略**，让宿主可以选择"陷阱交付后继续跑"。

最小侵入方案（建议）：

```rust
// 新增 Command（schema 版本 +1；默认值保持现有行为逐字节不变）
SetStopPolicy {
    stop_on_trap: bool,          // 默认 true
    max_trap_events: Option<u32>, // 默认 None = 不限；Some(n) 达上限后按 Trap 停
}
```

语义：

| `stop_on_trap` | 行为 |
| --- | --- |
| `true`（默认） | 与现在完全一致：`StepOutcome::Trapped` → 发 `Halted{reason:Trap}`，批次结束 |
| `false` | 陷阱仍按 M-13 交付（`enter_trap` 的一切实质行为不变），该条指令计入 `executed`，**不发 `Halted`**，继续执行 `mtvec` 处的 handler；可选发 `TrapDelivered{cause, mepc, mtvec_target, seq}` 事件（默认不订阅，供调试/统计） |

约束与建议：

- `HaltReason` 枚举**不要新增变体**（前端 JS 镜像硬编码了枚举，加会扩散改动）；
  `Trap` 的语义继续是"因陷阱而停"，只在 `stop_on_trap=false` 时不产生该停止。
- `max_trap_events` 存在的理由：handler 自身死循环时，不限流会把指令预算烧穿、
  失去诊断信息。达上限后按 `Halted{reason:Trap}` 停，并在事件里带 `trap_count`。
- `stop_on_trap=false` 时 `last_stop_reason()` 不应被写成 `Trap`，否则宿主无法区分
  "跑满预算"和"被陷阱停下"。

### 验收标准

- **AC1（回归）** 默认配置下，现有调试流程（断点 / 单步 / stepOver / 观察点）的
  事件序列与改动前逐字节一致；现有契约测试全绿。
- **AC2（对拍）** 同一份 ELF（OSZ `target_qemu_test`）在 sim（`stop_on_trap=false`）
  与 `qemu-system-riscv32` 上结论一致：8 条用例（7 条功能 + 1 条"故意野指针 →
  CRASH → 后续用例继续"）结果完全相同。
- **AC3（限流）** `max_trap_events = 1024`，构造一个 handler 内死循环的用例，
  应在 1024 个陷阱后停并报 `Halted{reason:Trap}`，不烧穿预算。
- **AC4（一致）** 若实现 `TrapDelivered`，其 `cause/mepc/mtvec_target` 必须与
  `enter_trap` 实际写入的值一致。

### 复现材料

```
镜像：build-test/targets/qemu_virt/test/target_qemu_test（ELF，229 KB）
命令：node tools/sim/run_on_platform.mjs <elf>        # RISCV_PREFIX 默认 "OSZ$ "
期望：console 出现 "[SZTEST] end fails=N"，而不是 halted reason=Trap
```

---

## R-2　CLINT `mtime` 的 64 位读偶发返回异常值

### 现象

同一个 session 内连续读 `0x200BFF8`（CLINT mtime）：

| 次序 | 读到的值 | 换算 |
| --- | --- | --- |
| 1 | `0x4E20`（20000 cycles） | 2 ms，正常 |
| 2 | `0x12A7D7FBECD0` | ≈ 2051 秒，异常 |
| 3 | `0x14888`（84104 cycles） | 8 ms，正常 |

我们实测位置：OSZ 测试套件里，一次 `drv_timer_get_time_stamp()`（=读 mtime 再除以
`TIMER_CYCLES_PER_US`）返回了 2051 秒的绝对时间，导致单条用例耗时显示 `2051209243 ms`。

### 期望

RV32 上没有原子 64 位 load，硬件经典做法是"读低半字时锁存高半字"。请让 guest
的 64 位 load 至少满足其一：

1. 原子快照（读低→读高→重读低，不一致则重试，对 guest 透明）；或
2. 按规范实现锁存（`mtime` 低字被读时锁存高字，直到高字被读走）。

### 验收标准

- 连续 1,000,000 次 64 位读，值单调不减；
- 相邻两次读的差值 < 1000 cycles（无跳变）；
- 与 R-1 无关，可独立合入。

---

## 优先级建议

R-1 建议尽快（它决定"能不能在 sim 上跑 OS"）；R-2 可以排在 R-1 之后，
但在 OSZ 进入性能测试（P2）之前必须解决，否则耗时指标不可信。
