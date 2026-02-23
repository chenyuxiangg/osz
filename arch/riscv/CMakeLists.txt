define_module(
    NAME arch_riscv
    TYPE STATIC
    SOURCES 
        src/csr.c
    ASM_SOURCES
        src/memcpy.S
        src/memset.S
        src/strlen.S
        src/strncmp.S
        src/strncpy.S
        src/udivdi3.S
        src/umoddi3.S
    PUBLIC_INCLUDES 
        ./include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
)