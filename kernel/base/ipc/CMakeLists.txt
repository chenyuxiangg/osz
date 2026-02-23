define_module(
    NAME k_base_ipc
    TYPE STATIC
    SOURCES 
        src/event/event.c
        src/mutex/mutex.c
        src/sem/sem.c
    PUBLIC_INCLUDES 
        ./include
        ./include/atomic
        ./include/event
        ./include/mutex
        ./include/sem
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_struct_dl
        k_base_task
        k_base_mem
)