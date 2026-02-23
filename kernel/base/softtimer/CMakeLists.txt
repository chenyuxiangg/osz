define_module(
    NAME k_base_swt
    TYPE STATIC
    SOURCES 
        src/softtime.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PUBLIC_DEPENDENCIES
        k_struct_sortlink
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_drv_timer
        k_base_task
        k_base_mem
        k_base_ipc
)