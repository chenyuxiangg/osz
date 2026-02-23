#include "comm.h"
#include "exception.h"
#include "csr.h"
#include "task.h"

UINT32 exception_handler(UINT32 mcause, UINT32 mepc)
{
    printf("mcause: %#x, mepc: %#x\n", mcause, mepc);
    printf("mtval: %#x\n", CSR_READ(mtval));
    uint16_t tid = osz_get_current_tid();
    if (tid < OSZ_CFG_TASK_LIMIT) {
        printf("current task: [%d]%s\n", tid, TASK_NAME(tid));
    }
    while(1);
    return (mepc);
}