# inherit from build.sh
user_home="${HOME:-$(getent passwd $USER | cut -d: -f6)}"
global_toolchain_path=${user_home}/tools/toolchains
compiler_name=riscv32-unknown-elf

# Determine paths relative to this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONSTRUCT_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$CONSTRUCT_DIR")"

tools_path=${PROJECT_ROOT}/tools
build_path=${CONSTRUCT_DIR}
qemu_path=${tools_path}/qemu
toolchain_path=${tools_path}/toolchain
compiler_path=${tools_path}/toolchain/${compiler_name}

config_process() {
    if [ ! -L ${toolchain_path} ]; then
        ln -s ${global_toolchain_path} ${toolchain_path}
    fi
}

config_toolchain() {
    if [ ! -d ${qemu_path} ]; then
        if [ ! -f ${qemu_path}.tar.gz ]; then
            echo "Error: not fount ${qemu_path}.tar.gz"
            exit -1
        fi
        pushd ${tools_path}
        tar xz -f ${qemu_path}.tar.gz
        popd
    fi

    if [ ! -d ${compiler_path} ]; then
        if [ ! -f ${compiler_path}.tar.gz ]; then
            echo "Error: not fount ${compiler_path}.tar.gz"
            exit -1
        fi
        pushd ${toolchain_path}
        tar xz -f ${compiler_path}.tar.gz
        popd
    fi
}

set_env() {
    export CROSS_COMPILER_PATH=${compiler_path}/bin
    export QEMU_PATH=${tools_path}/qemu
    export COMM_MK=${build_path}/make/comm.mk
    export CFG_MK=${build_path}/make/cfg.mk
    export MODULE_MK=${build_path}/make/module.mk
    export OUTPUT_PATH=${PROJECT_ROOT}/output
}

gen_config() {
    python3 ${build_path}/script/parser_cfg.py
}

config_process
config_toolchain
# gen_config   # Now handled by Makefile's gen_header target
set_env
