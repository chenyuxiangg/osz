define_module(
    NAME k_struct_fifo
    TYPE STATIC
    SOURCES 
        src/fifo.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_base_mem
        k_base_ipc
)