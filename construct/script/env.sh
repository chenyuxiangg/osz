#!/bin/bash
# ============================================================================
# OSZ 构建环境解析（CMake / Makefile 两条构建路径共用同一套查找规则）
#
# 工具链查找顺序，与 construct/cmake/riscv32_toolchain.cmake 保持一致：
#   1. 显式指定：OSZ_CROSS_ROOT、OSZ_CROSS_PREFIX
#   2. 仓库 tools/toolchain（可选目录或符号链接，不入 git）
#   3. 旧路径 ${HOME}/tools/toolchains（存在 <prefix>.tar.gz 时自动解压）
#   4. 系统 PATH 中的 <prefix>-gcc
#
# 换机器只需 export 两个变量，或把工具链放进 tools/toolchain，
# 不需要修改仓库内的任何文件。
# ============================================================================

user_home="${HOME:-$(getent passwd "$USER" | cut -d: -f6)}"
global_toolchain_path="${user_home}/tools/toolchains"

# Determine paths relative to this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONSTRUCT_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$CONSTRUCT_DIR")"

tools_path=${PROJECT_ROOT}/tools
build_path=${CONSTRUCT_DIR}
qemu_path=${QEMU_PATH:-${tools_path}/qemu}
toolchain_path=${tools_path}/toolchain

# 交叉编译目标参数，可用环境变量覆盖（与 CMake 侧同名）
osz_cross_arch=${OSZ_CROSS_ARCH:-rv32g}
osz_cross_abi=${OSZ_CROSS_ABI:-ilp32d}

# ---------------------------------------------------------------------------
# 工具链前缀候选：显式指定优先，其余按发行包常见命名依次尝试
# ---------------------------------------------------------------------------
prefix_candidates() {
    [ -n "${OSZ_CROSS_PREFIX}" ] && printf '%s\n' "${OSZ_CROSS_PREFIX}"
    printf '%s\n' riscv32-unknown-elf riscv-none-elf riscv64-unknown-elf
}

# 在根目录下定位 <prefix>-gcc，支持 <root>/bin 与 <root>/<prefix>/bin 两种布局
# 成功时输出 bin 目录
gcc_bindir_in_root() {
    local root=$1 prefix=$2 candidate
    for candidate in "${root}/bin" "${root}/${prefix}/bin"; do
        if [ -x "${candidate}/${prefix}-gcc" ]; then
            printf '%s' "${candidate}"
            return 0
        fi
    done
    return 1
}

# 在候选根目录集合中按前缀顺序查找，最后回退到 PATH
search_toolchain() {
    local root prefix bindir

    for root in "$@"; do
        [ -n "${root}" ] || continue
        [ -d "${root}" ] || continue
        for prefix in $(prefix_candidates); do
            if bindir=$(gcc_bindir_in_root "${root}" "${prefix}"); then
                CROSS_COMPILER_PREFIX=${prefix}
                CROSS_COMPILER_PATH=${bindir}
                return 0
            fi
        done
    done

    for prefix in $(prefix_candidates); do
        if command -v "${prefix}-gcc" >/dev/null 2>&1; then
            CROSS_COMPILER_PREFIX=${prefix}
            CROSS_COMPILER_PATH="$(dirname "$(command -v "${prefix}-gcc")")"
            return 0
        fi
    done

    return 1
}

# 旧路径兼容：${HOME}/tools/toolchains/<prefix>.tar.gz 存在时解压到同目录
legacy_extract_toolchain() {
    local prefix tarball
    for prefix in $(prefix_candidates); do
        tarball="${global_toolchain_path}/${prefix}.tar.gz"
        if [ -f "${tarball}" ] && [ ! -x "${global_toolchain_path}/${prefix}/bin/${prefix}-gcc" ]; then
            echo "[Info]	解压旧路径工具链 ${tarball}"
            mkdir -p "${global_toolchain_path}"
            tar xzf "${tarball}" -C "${global_toolchain_path}" || return 1
        fi
    done
    return 0
}

resolve_toolchain() {
    search_toolchain "${OSZ_CROSS_ROOT:-}" "${toolchain_path}" "${global_toolchain_path}" && return 0
    legacy_extract_toolchain
    search_toolchain "${OSZ_CROSS_ROOT:-}" "${toolchain_path}" "${global_toolchain_path}"
}

# ---------------------------------------------------------------------------
# QEMU：只有 run/debug 类目标需要，缺失时不阻断编译
# ---------------------------------------------------------------------------
config_qemu() {
    if [ -d "${qemu_path}" ]; then
        :
    elif [ -f "${qemu_path}.tar.gz" ] && [ "$(uname -s)" = "Linux" ]; then
        # 仓库内自带的 qemu 包是 Linux/x86_64 版本，只在 Linux 主机上解压
        echo "[Info]	解压 ${qemu_path}.tar.gz"
        tar xzf "${qemu_path}.tar.gz" -C "${tools_path}" || \
            echo "[Warn]	${qemu_path}.tar.gz 解压失败，run/debug 目标将不可用"
    elif [ -f "${qemu_path}.tar.gz" ]; then
        echo "[Info]	$(uname -s) 主机不解压自带的 Linux 版 qemu 包；如需 run/debug 请自行提供匹配平台的 ${qemu_path}/bin/qemu-system-riscv32"
    fi

    if [ ! -x "${qemu_path}/bin/qemu-system-riscv32" ]; then
        echo "[Warn]	未找到 qemu-system-riscv32（${qemu_path}），run/debug 目标不可用；编译不受影响"
    fi
    return 0
}

# 工具链自检：编译器存在，且目标参数被接受
check_toolchain() {
    local cc="${CROSS_COMPILER_PATH}/${CROSS_COMPILER_PREFIX}-gcc"

    if [ ! -x "${cc}" ]; then
        echo "Error: 未找到 RISC-V 交叉工具链，已尝试以下来源："
        echo "        1) OSZ_CROSS_ROOT=${OSZ_CROSS_ROOT:-<未设置>}  OSZ_CROSS_PREFIX=${OSZ_CROSS_PREFIX:-<自动探测>}"
        echo "        2) ${toolchain_path}"
        echo "        3) ${global_toolchain_path}"
        echo "        4) 系统 PATH"
        echo "       请执行 export OSZ_CROSS_ROOT=<工具链根目录> OSZ_CROSS_PREFIX=<前缀>，"
        echo "       或把工具链目录/符号链接放到 ${toolchain_path}（该路径已 git-ignore）。"
        exit 1
    fi

    if ! printf 'int osz_cc_probe(void){return 0;}\n' |
        "${cc}" -march="${osz_cross_arch}" -mabi="${osz_cross_abi}" -ffreestanding \
            -x c -c -o /dev/null - 2>/dev/null; then
        echo "Error: 工具链 ${cc} 不接受 -march=${osz_cross_arch} -mabi=${osz_cross_abi}"
        echo "       请用 OSZ_CROSS_ARCH / OSZ_CROSS_ABI 指定该工具链支持的组合。"
        exit 1
    fi
}

set_env() {
    export CROSS_COMPILER_PREFIX
    export CROSS_COMPILER_PATH
    export OSZ_CROSS_ARCH="${osz_cross_arch}"
    export OSZ_CROSS_ABI="${osz_cross_abi}"
    export QEMU_PATH="${qemu_path}"
    export COMM_MK="${build_path}/make/comm.mk"
    export CFG_MK="${build_path}/make/cfg.mk"
    export MODULE_MK="${build_path}/make/module.mk"
    export OUTPUT_PATH="${PROJECT_ROOT}/output"
}

gen_config() {
    python3 "${build_path}/script/parser_cfg.py"
}

if ! resolve_toolchain; then
    CROSS_COMPILER_PREFIX=riscv32-unknown-elf
    CROSS_COMPILER_PATH="${toolchain_path}/${CROSS_COMPILER_PREFIX}/bin"
fi

check_toolchain
config_qemu
# gen_config   # Now handled by Makefile's gen_header target
set_env
