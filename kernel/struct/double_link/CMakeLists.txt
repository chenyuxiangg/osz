define_module(
    NAME k_struct_dl
    TYPE STATIC
    SOURCES 
        src/dlink.c
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
)