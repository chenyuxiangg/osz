define_module(
    NAME k_debug
    TYPE STATIC
    SOURCES 
        src/printf.c
        src/putchar.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        k_drv_uart
)