define_module(
    NAME k_drv_timer
    TYPE STATIC
    SOURCES 
        src/ext/timer.c
        src/hal/hal_clint_timer.c
    PUBLIC_INCLUDES 
        ./include/ext
        ./include/hal
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PUBLIC_DEPENDENCIES
        k_base_trap
        k_struct_dl
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_base_task
        k_struct_sortlink
        k_base_swt
)