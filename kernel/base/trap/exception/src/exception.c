#include "comm.h"
#include "exception.h"
#include "csr.h"
#include "task.h"

#if defined(OSZ_CFG_TEST) && OSZ_CFG_TEST
/* 测试构建：同步异常可以归因到具体用例并恢复，其余行为保持不变 */
/* 弱符号：未链接测试 harness 的镜像（如产品镜像）自动回退到原有的打印+停机行为 */
extern void tc_exception_recover(uint32_t mcause, uint32_t mepc, uint32_t mtval)
    __attribute__((noreturn, weak));
#endif

UINT32 exception_handler(UINT32 mcause, UINT32 mepc)
{
#if defined(OSZ_CFG_TEST) && OSZ_CFG_TEST
    if (tc_exception_recover != 0) {
        tc_exception_recover(mcause, mepc, (uint32_t)CSR_READ(mtval));
    }
#endif
    printf("mcause: %#x, mepc: %#x\n", mcause, mepc);
    printf("mtval: %#x\n", CSR_READ(mtval));
    uint16_t tid = osz_get_current_tid();
    if (tid < OSZ_CFG_TASK_LIMIT) {
        printf("current task: [%d]%s\n", tid, TASK_NAME(tid));
    }
    while(1);
    return (mepc);
}