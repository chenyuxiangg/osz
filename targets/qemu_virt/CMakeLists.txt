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
)