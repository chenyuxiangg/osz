define_module(
    NAME k_comm
    TYPE STATIC
    SOURCES 
        src/object.c
        src/strtoul.c
    ASM_SOURCES
        src/assert.S
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PUBLIC_DEPENDENCIES
        k_struct_dl
)