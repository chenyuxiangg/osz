#!/usr/bin/env python3
"""板上系统级测试的宿主侧 runner。

职责边界（刻意保持很薄）：
  1) 把测试镜像跑起来（本机 wasm 仿真平台 / QEMU virt 二选一）；
  2) 收集 console 字节流，按 `[SZTEST] v1` 行协议解析成结构化结果；
  3) 产出 result.json + junit.xml，并用进程退出码表达结论。

它不认识任何用例，也不做任何断言判断——判定只发生在板上。

用法：
    python3 test/sys/runner/run_systest.py \
        --backend sim \
        --elf build-test/targets/qemu_virt/test/target_qemu_test

    python3 test/sys/runner/run_systest.py --backend qemu --elf <elf> \
        --qemu qemu-system-riscv32

退出码：0 全通过；1 有 FAIL/CRASH/SKIP 之外的不通过；2 runner 自身或环境问题。
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional

REPO_ROOT = Path(__file__).resolve().parents[3]

SUITE_START = re.compile(r"^\[SZTEST\] v(\d+) suite_start total=(\d+)(?: board=(\S+))?")
CASE_BEGIN = re.compile(r"^\[SZTEST\] case_begin (\S+)")
CASE_END = re.compile(r"^\[SZTEST\] case_end (\S+) (\w+) ms=(\d+) asserts=(\d+)")
ASSERT_FAIL = re.compile(r'^\[SZTEST\] assert_fail (.+?):(\d+) "(.*)"$')
ASSERT_DETAIL = re.compile(
    r"^\[SZTEST\] assert_detail (.+?):(\d+) expect=(0x[0-9a-fA-F]+|\d+) actual=(0x[0-9a-fA-F]+|\d+)"
)
CASE_SKIP = re.compile(r"^\[SZTEST\] case_skip (\S+)(?: \"(.*)\")?")
SUITE_END = re.compile(
    r"^\[SZTEST\] suite_end pass=(\d+) fail=(\d+) crash=(\d+) skip=(\d+) ms=(\d+)"
)
SENTINEL = re.compile(r"^\[SZTEST\] sentinel (OK|FAILED)$")
TAIL_FAILS = re.compile(r"^\[SZTEST\] end fails=(\d+)")


@dataclass
class Failure:
    file: str = ""
    line: int = 0
    expr: str = ""
    expect: Optional[str] = None
    actual: Optional[str] = None


@dataclass
class Case:
    name: str
    status: str = "UNKNOWN"
    ms: int = 0
    asserts: int = 0
    failures: List[Failure] = field(default_factory=list)


@dataclass
class Suite:
    protocol: int = 0
    total: int = 0
    board: str = ""
    pass_: int = 0
    fail: int = 0
    crash: int = 0
    skip: int = 0
    ms: int = 0
    sentinel: str = ""
    cases: List[Case] = field(default_factory=list)


def parse_console(text: str) -> Suite:
    suite = Suite()
    current: Optional[Case] = None
    for raw in text.splitlines():
        line = raw.strip().replace("\r", "")
        # shell 提示符会和日志挤在同一行（"OSZ$ [SZTEST] ..."），先切到协议起点
        idx = line.find("[SZTEST]")
        if idx < 0:
            continue
        line = line[idx:]
        m = SUITE_START.match(line)
        if m:
            suite.protocol = int(m.group(1))
            suite.total = int(m.group(2))
            suite.board = m.group(3) or ""
            continue
        m = CASE_BEGIN.match(line)
        if m:
            current = Case(name=m.group(1))
            suite.cases.append(current)
            continue
        m = CASE_END.match(line)
        if m:
            name, status, ms, asserts = m.group(1), m.group(2), int(m.group(3)), int(m.group(4))
            if current is None or current.name != name:
                current = Case(name=name)
                suite.cases.append(current)
            current.status = status
            current.ms = ms
            current.asserts = asserts
            continue
        m = ASSERT_FAIL.match(line)
        if m and current is not None:
            current.failures.append(Failure(file=m.group(1), line=int(m.group(2)), expr=m.group(3)))
            continue
        m = ASSERT_DETAIL.match(line)
        if m and current is not None and current.failures:
            current.failures[-1].expect = m.group(3)
            current.failures[-1].actual = m.group(4)
            continue
        m = CASE_SKIP.match(line)
        if m and current is not None:
            current.status = "SKIP"
            continue
        m = SUITE_END.match(line)
        if m:
            suite.pass_ = int(m.group(1))
            suite.fail = int(m.group(2))
            suite.crash = int(m.group(3))
            suite.skip = int(m.group(4))
            suite.ms = int(m.group(5))
            continue
        m = SENTINEL.match(line)
        if m:
            suite.sentinel = m.group(1)
            continue
    return suite


# ----------------------------- backends -------------------------------------

def run_sim(elf: Path, timeout: int, extra_env: dict) -> tuple[int, str, str]:
    runner = REPO_ROOT / "tools/sim/run_on_platform.mjs"
    if not runner.exists():
        raise RuntimeError(f"找不到仿真平台 runner: {runner}")
    env = dict(os.environ)
    env.setdefault("RISCV_PREFIX", "[SZTEST] end")
    env.update(extra_env)
    proc = subprocess.run(
        ["node", str(runner), str(elf)],
        cwd=str(REPO_ROOT),
        env=env,
        capture_output=True,
        timeout=timeout,
    )
    return proc.returncode, proc.stdout.decode("utf-8", "replace"), proc.stderr.decode("utf-8", "replace")


def run_qemu(elf: Path, timeout: int, qemu: str, extra_env: dict) -> tuple[int, str, str]:
    exe = shutil.which(qemu) or (qemu if Path(qemu).is_file() else None)
    if exe is None:
        raise RuntimeError(
            "找不到 qemu-system-riscv32。仓库 tools/qemu 尚未解压，或本机未安装；"
            "可先用 --backend sim 跑同一份 ELF。"
        )
    proc = subprocess.run(
        [str(exe), "-machine", "virt", "-bios", "none", "-kernel", str(elf), "-nographic"],
        cwd=str(REPO_ROOT),
        env={**os.environ, **extra_env},
        capture_output=True,
        timeout=timeout,
    )
    return proc.returncode, proc.stdout.decode("utf-8", "replace"), proc.stderr.decode("utf-8", "replace")


# ----------------------------- reporting ------------------------------------

def write_outputs(out_dir: Path, suite: Suite, meta: dict, raw: str) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "console.log").write_text(raw, encoding="utf-8")

    result = {
        "schema": 1,
        "runner": meta,
        "board": suite.board,
        "suite": {
            "total": suite.total,
            "pass": suite.pass_,
            "fail": suite.fail,
            "crash": suite.crash,
            "skip": suite.skip,
            "ms": suite.ms,
            "sentinel": suite.sentinel,
        },
        "cases": [
            {
                "name": c.name,
                "status": c.status,
                "ms": c.ms,
                "asserts": c.asserts,
                "failures": [f.__dict__ for f in c.failures],
            }
            for c in suite.cases
        ],
    }
    (out_dir / "result.json").write_text(json.dumps(result, indent=2, ensure_ascii=False), encoding="utf-8")

    total = suite.total or len(suite.cases)
    bad = suite.fail + suite.crash
    lines = [
        f'<?xml version="1.0" encoding="UTF-8"?>',
        f'<testsuite name="osz-systest-{suite.board or "board"}" tests="{total}" '
        f'failures="{bad}" skipped="{suite.skip}" time="{suite.ms / 1000.0:.3f}">',
    ]
    for c in suite.cases:
        lines.append(f'  <testcase classname="osz.systest" name="{c.name}" time="{c.ms / 1000.0:.3f}">')
        if c.status in ("FAIL", "CRASH"):
            msg = c.failures[0].expr if c.failures else c.status
            body = "\n".join(
                f'{f.file}:{f.line} "{f.expr}" expect={f.expect} actual={f.actual}' for f in c.failures
            )
            lines.append(f'    <failure message="{escape_xml(msg)}">{escape_xml(body)}</failure>')
        elif c.status == "SKIP":
            lines.append("    <skipped />")
        lines.append("  </testcase>")
    lines.append("</testsuite>")
    (out_dir / "junit.xml").write_text("\n".join(lines) + "\n", encoding="utf-8")


def escape_xml(s: str) -> str:
    return (
        s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")
    )


def main() -> int:
    ap = argparse.ArgumentParser(description="OSZ 板上系统级测试 runner")
    ap.add_argument("--backend", choices=["sim", "qemu"], default="sim")
    ap.add_argument("--elf", required=True, help="测试镜像 ELF 路径")
    ap.add_argument("--qemu", default="qemu-system-riscv32", help="qemu 可执行文件或绝对路径")
    ap.add_argument("--out", default="build-test/systest-out", help="结果输出目录")
    ap.add_argument("--timeout", type=int, default=180, help="整体超时（秒）")
    ap.add_argument("--allow-skip", action="store_true", help="SKIP 不算失败")
    args = ap.parse_args()

    elf = Path(args.elf)
    if not elf.is_absolute():
        elf = (REPO_ROOT / elf).resolve()
    if not elf.exists():
        print(f"[runner] ELF 不存在: {elf}", file=sys.stderr)
        return 2

    out_dir = Path(args.out)
    if not out_dir.is_absolute():
        out_dir = (REPO_ROOT / out_dir).resolve()

    started = time.time()
    try:
        if args.backend == "sim":
            rc, out, err = run_sim(elf, args.timeout, {})
        else:
            rc, out, err = run_qemu(elf, args.timeout, args.qemu, {})
    except subprocess.TimeoutExpired:
        print(f"[runner] 超时 {args.timeout}s", file=sys.stderr)
        return 2
    except RuntimeError as exc:
        print(f"[runner] {exc}", file=sys.stderr)
        return 2

    raw = out
    if err:
        print(err, file=sys.stderr, end="")

    suite = parse_console(raw)
    meta = {
        "backend": args.backend,
        "elf": str(elf),
        "host_rc": rc,
        "host_elapsed_s": round(time.time() - started, 3),
    }
    write_outputs(out_dir, suite, meta, raw)

    # 判定：必须有 sentinel 且失败数为 0，才算这套跑完整了
    if suite.sentinel == "":
        print("[runner] 控制台里没有 sentinel：套件没跑完（可能早期 trap/超时）", file=sys.stderr)
        print(f"[runner] 原始输出已写 {out_dir / 'console.log'}", file=sys.stderr)
        return 2

    bad = suite.fail + suite.crash + (0 if args.allow_skip else suite.skip)
    for c in suite.cases:
        mark = {"PASS": "✓", "FAIL": "✗", "CRASH": "✗", "SKIP": "-"}.get(c.status, "?")
        print(f"  {mark} {c.name:<32} {c.status:<6} asserts={c.asserts} ms={c.ms}")
        for f in c.failures[:3]:
            print(f"      {f.file}:{f.line} \"{f.expr}\" expect={f.expect} actual={f.actual}")

    print(
        f"[runner] total={suite.total} pass={suite.pass_} fail={suite.fail} "
        f"crash={suite.crash} skip={suite.skip} ms={suite.ms} -> {out_dir}"
    )
    return 0 if bad == 0 and suite.sentinel == "OK" else 1


if __name__ == "__main__":
    sys.exit(main())
