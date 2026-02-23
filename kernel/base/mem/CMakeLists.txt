define_module(
    NAME k_base_mem
    TYPE STATIC
    SOURCES 
        bestfit/src/bestfit.c
        src/mem.c
    PUBLIC_INCLUDES 
        ./include
        ./bestfit/include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PUBLIC_DEPENDENCIES
        k_struct_dl
)