// run_on_platform.mjs —— 把 osz 的 ELF 放到本机 RISC-V 仿真平台的 wasm 内核上跑。
//
// 复用 8080 工作台**同一份** wasm 产物（`front/assets/pkg/riscv_sim_kernel.js`），
// 不启动浏览器、不改仿真平台仓库：一条命令拿到 console 字节流 + 停机时的 PC。
//
// 用法：
//   node tools/sim/run_on_platform.mjs <elf> [指令预算]
//
// 环境变量：
//   RISCV_SIM_DIR   仿真平台仓库根目录（默认 ~/lczyzs/code/riscv_sim）
//   RISCV_SIM_BOARD 板模板 id（默认 qemu-virt，须与 osz 的 platform 配置一致）
//   RISCV_PREFIX    视为「启动成功」的 console 前缀（默认 "OSZ$ "）
//
// 前置条件（v0.76 起平台侧的纪律）：装载前**必须**先配一块板，否则内核以
// `E_PLT-22` 拒绝装载 —— 本脚本按「取模板 → SetMemoryMap → LoadProject」
// 的顺序走，就是为了满足这条纪律。
//
// 输出：console 原始字节写 stdout（可与 Spike 的 stdout 逐字节 cmp），
//       运行统计写 stderr。

import { readFileSync } from "node:fs";
import { pathToFileURL } from "node:url";
import { homedir } from "node:os";
import { join } from "node:path";

const SIM_DIR = process.env.RISCV_SIM_DIR || join(homedir(), "lczyzs", "code", "riscv_sim");
const PKG = join(SIM_DIR, "front/assets/pkg/riscv_sim_kernel.js");
const WASM = join(SIM_DIR, "front/assets/pkg/riscv_sim_kernel_bg.wasm");
const BOARD = process.env.RISCV_SIM_BOARD || "qemu-virt";
const PREFIX = process.env.RISCV_PREFIX ?? "OSZ$ ";

const elfPath = process.argv[2];
if (!elfPath) {
    console.error("用法: node tools/sim/run_on_platform.mjs <elf> [指令预算]");
    process.exit(1);
}
const MAX_N = Number(process.argv[3] || 30_000_000);
const CHUNK = 500_000;

const mod = await import(pathToFileURL(PKG).href);
mod.initSync({ module: new WebAssembly.Module(readFileSync(WASM)) });
const schemaVersion = mod.protocol_schema_version();
console.error(`[sim] kernel=${mod.kernel_version()} schema=${schemaVersion} board=${BOARD}`);

let seq = 0;
const consoleBytes = [];
const errors = [];
let lastPc = null;
let halted = null;

function send(payload) {
    seq += 1;
    const raw = mod.process_message(
        JSON.stringify({
            schema_version: schemaVersion,
            direction: "Command",
            seq,
            payload: { Command: payload },
        }),
    );
    const events = JSON.parse(raw);
    for (const ev of events) {
        const p = ev?.payload?.Event;
        if (!p || typeof p !== "object") continue;
        if (p.Console && Array.isArray(p.Console.bytes)) consoleBytes.push(...p.Console.bytes);
        if (p.Error) errors.push(p.Error);
        if (p.SimStateDelta && typeof p.SimStateDelta.pc === "number") lastPc = p.SimStateDelta.pc;
        if (p.Halted) halted = p.Halted;
    }
    return events;
}

// 1) 板模板 → 配板
const tpl = send({ GetPlatformTemplate: { id: BOARD } });
let description = null;
for (const ev of tpl) {
    const p = ev?.payload?.Event;
    if (p?.PlatformTemplate?.description) description = p.PlatformTemplate.description;
}
if (!description) {
    console.error(`[sim] 拿不到板模板 ${BOARD} 的描述`);
    process.exit(1);
}
console.error(`[sim] 板=${description.id} isa=${description.mcu?.isa ?? "-"} regions=${description.regions.length}`);
send({ SetMemoryMap: { description, expected_revision: null } });

// 2) 装载 ELF
const bytes = Array.from(readFileSync(elfPath));
send({
    LoadProject: {
        id: "osz-sim-run",
        name: "osz",
        tree: { root: { name: "osz", component_id: "rv-sim", children: [] } },
        images: [{ name: "osz.elf", kind: "Elf", source: { Bytes: bytes } }],
        version: { schema: 1, isa: description.mcu?.isa ?? "rv32imac", toolchain: "xpack riscv-none-elf 15.2.0" },
    },
});
if (errors.length) {
    console.error(`[sim] 装载失败: ${JSON.stringify(errors.slice(0, 2))}`);
    process.exit(3);
}

// 3) 订阅（LoadProject 会清订阅表，必须在其之后订）
send({ Subscribe: { topic: "sim.console" } });
send({ Subscribe: { topic: "sim.state.delta" } });

// 4) 步进到提示符或预算耗尽
const t0 = Date.now();
let executed = 0;
while (executed < MAX_N) {
    const n = Math.min(CHUNK, MAX_N - executed);
    send({ StepN: { n } });
    executed += n;
    if (Buffer.from(consoleBytes).toString("utf8").includes(PREFIX)) break;
    if (errors.length || halted) break;
}

const text = Buffer.from(consoleBytes).toString("utf8");
process.stdout.write(text);
const pc = lastPc === null ? "-" : `0x${lastPc.toString(16).padStart(8, "0")}`;
console.error(
    `[sim] 执行 ${executed} 条 / ${Date.now() - t0} ms / console ${consoleBytes.length} 字节 / 命中前缀=${text.includes(PREFIX)} / PC=${pc}`,
);
if (halted) console.error(`[sim] halted: ${JSON.stringify(halted)}`);
if (errors.length) console.error(`[sim] 内核错误: ${JSON.stringify(errors.slice(0, 3))}`);
