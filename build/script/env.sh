# inherit from build.sh
compiler_name=riscv32-unknown-elf
tools_path=${current_path}/../tools
build_path=${current_path}
qemu_path=${tools_path}/qemu
toolchain_path=${tools_path}/${compiler_name}

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
    if [ ! -d ${toolchain_path} ]; then
        if [ ! -f ${toolchain_path}.tar.gz ]; then
            echo "Error: not fount ${toolchain_path}.tar.gz"
            exit -1
        fi
        pushd ${tools_path}
        tar xz -f ${toolchain_path}.tar.gz
        popd
    fi
}

set_env() {
    export CROSS_COMPILER_PATH=${tools_path}/${compiler_name}/bin
    export QEMU_PATH=${tools_path}/qemu
    export COMM_MK=${build_path}/make/comm.mk
    export CFG_MK=${build_path}/make/cfg.mk
    export MODULE_MK=${build_path}/make/module.mk
    export OUTPUT_PATH=${current_path}/../output
}

gen_config() {
    python3 ${build_path}/script/parser_cfg.py
}

config_toolchain
gen_config
set_env