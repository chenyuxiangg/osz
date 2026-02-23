define_module(
    NAME k_drv_uart
    TYPE STATIC
    SOURCES 
        src/ext/uart.c
        src/hal/hal_uart.c
    PUBLIC_INCLUDES 
        ./include/ext
        ./include/hal
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PUBLIC_DEPENDENCIES
        k_base_trap
)