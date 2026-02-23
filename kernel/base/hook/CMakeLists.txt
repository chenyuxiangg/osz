define_module(
    NAME k_base_hook
    TYPE STATIC
    SOURCES 
        src/hook.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
)