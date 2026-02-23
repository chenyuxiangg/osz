define_module(
    NAME k_shell
    TYPE STATIC
    SOURCES 
        src/shell.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_COMPILE_OPTIONS
        -Wno-incompatible-pointer-types
    PUBLIC_DEPENDENCIES
        k_struct_fifo
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_base_task
        k_base_mem
)