define_module(
    NAME k_dfx
    TYPE STATIC
    SOURCES 
        src/task/dfx_task.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_base_task
        k_base_hook
        k_shell
)