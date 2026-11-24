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
        ${OSZ_KERNEL_MODULES}
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

# 板上系统测试镜像（OSZ_TEST_BUILD=ON 时才有意义）
if(OSZ_TEST_BUILD)
    add_subdirectory(test)
endif()
