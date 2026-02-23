define_module(
    NAME k_base_sche
    TYPE STATIC
    SOURCES 
        preemption/priority_queue/src/pri_queue.c
        preemption/src/preemption.c
    ASM_SOURCES
        src/schedule.S
    PUBLIC_INCLUDES 
        ./include
        ./preemption/include
        ./preemption/priority_queue/include
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PUBLIC_DEPENDENCIES
        k_struct_dl
    PRIVATE_DEPENDENCIES
        arch_riscv
        k_base_task
)