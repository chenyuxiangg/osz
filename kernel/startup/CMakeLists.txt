define_module(
    NAME k_startup
    TYPE STATIC
    SOURCES 
    ASM_SOURCES
        start.S
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
)