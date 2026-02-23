define_module(
    NAME target_qemu
    TYPE EXECUTABLE
    SOURCES 
        src/main.c
    DISASSEMBLE_TOOL
        ${CMAKE_OBJDUMP}
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_startup
        k_base_init
        k_base_task
        k_base_hook
        k_base_sche
        k_base_mem
        k_base_ipc
        k_shell
        k_debug
        k_drv_uart
        k_comm
    # QEMU配置
    ENABLE_QEMU_RUN ON
    QEMU_EXECUTABLE ${OSZ_TOOLS_DIR}/qemu/bin/qemu-system-riscv32
    QEMU_MACHINE virt
    QEMU_CPU rv32
    QEMU_OPTIONS
        -nographic
        -bios none
    # GDB配置
    ENABLE_GDB_DEBUG ON
    GDB_EXECUTABLE ${OSZ_TOOLS_DIR}/toolchain/riscv32-unknown-elf/bin/riscv32-unknown-elf-gdb
    GDB_PORT 1234
    GDB_SCRIPT ${OSZ_BUILD_DIR}/script/gdbinit
)