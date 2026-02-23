define_module(
    NAME k_base_task
    TYPE STATIC
    SOURCES 
        src/task.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PUBLIC_DEPENDENCIES
        k_struct_dl
        k_struct_sortlink
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_base_sche
        k_base_mem
        k_drv_timer
)