define_module(
    NAME k_base_trap
    TYPE STATIC
    SOURCES 
        exception/src/exception.c
        interrupt/src/interrupt.c
    ASM_SOURCES
        src/trap_entry.S
    PUBLIC_INCLUDES 
        ./include
        ./exception/include
        ./interrupt/include
        ./interrupt/include/clint
        ./interrupt/include/plic
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_base_task
)