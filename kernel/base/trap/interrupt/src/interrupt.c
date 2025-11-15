#include "comm.h"
#include "platform.h"
#include "interrupt.h"
#include "plic.h"
#include "clint.h"
#include "csr.h"

STATIC INT_INFO g_int_infos[OSZ_CFG_INT_LIMIT + 1];

STATIC UINT32 get_context_id(VOID)
{
    return PLIC_HART0_MACHINE;
}

void interrupt_handler(UINT32 mcause)
{
    UINT32 source_id = mcause & MCAUSE_MASK_ECODE;
    if (source_id == M_EXTERNAL_INTERRUPT) {
        PLIC_GET_IP_AND_CLEAN(PLIC_BASE_ADDR, get_context_id(), source_id);
        source_id += LOCAL_INTERRUPT_MAX_NUM;
    }

    if (HAS_INT_CALLBACK(source_id)) {
        INT_CALLBACK_ENTITY(source_id)(NULL);
    }

    PLIC_NOTIFY_COMPLETE(PLIC_BASE_ADDR, get_context_id(), source_id - LOCAL_INTERRUPT_MAX_NUM);
}

UINT32 osz_interrupt_init(VOID)
{
    CSR_WRITE(mie, (1 << 11));
    osz_interrupt_set_pri_th(0);
    for (int i = 0; i < OSZ_CFG_INT_LIMIT; ++i) {
        g_int_infos[i].func = NULL;
        g_int_infos[i].args = NULL;
    }
    return OS_OK;
}
MODULE_INIT(osz_interrupt_init, l0)

UINT32 osz_interrupt_regist(UINT32 source_id, interrupt_callback func)
{
    ASSERT(func);

    if (source_id >= OSZ_CFG_INT_LIMIT) {
        return PLIC_ERRNO_INVALID_SOURCE_ID;
    }

    // PLIC 全局控制器的0号中断为无效中断
    if (source_id == LOCAL_INTERRUPT_MAX_NUM) {
        return PLIC_ERRNO_SOURCE_ID_IS_ZERO;
    }
    g_int_infos[source_id].func = func;
    return OS_OK;
}

UINT32 osz_interrupt_unregist(UINT32 source_id)
{
    if (source_id >= OSZ_CFG_INT_LIMIT) {
        return PLIC_ERRNO_INVALID_SOURCE_ID;
    }

    // PLIC 全局控制器的0号中断为无效中断
    if (source_id == LOCAL_INTERRUPT_MAX_NUM) {
        return PLIC_ERRNO_SOURCE_ID_IS_ZERO;
    }
    g_int_infos[source_id].func = NULL;
    g_int_infos[source_id].args = NULL;
    return OS_OK;
}

UINT32 osz_interrupt_enable(UINT32 source_id)
{
    if (source_id >= OSZ_CFG_INT_LIMIT) {
        return PLIC_ERRNO_INVALID_SOURCE_ID;
    }
    if (INT_IS_TIMER(source_id) || INT_IS_SOFTWARE(source_id)) {
        w_mie(r_mie() | (1 << source_id));
    } else {
        source_id -= LOCAL_INTERRUPT_MAX_NUM;
        // PLIC 全局控制器的0号中断为无效中断
        if (source_id == LOCAL_INTERRUPT_MAX_NUM) {
            return PLIC_ERRNO_SOURCE_ID_IS_ZERO;
        }
        PLIC_SET_IE(PLIC_BASE_ADDR, get_context_id(), source_id);
    }
    return OS_OK;
}

UINT32 osz_interrupt_disable(UINT32 source_id)
{
    if (source_id >= OSZ_CFG_INT_LIMIT) {
        return PLIC_ERRNO_INVALID_SOURCE_ID;
    }
    if (INT_IS_TIMER(source_id) || INT_IS_SOFTWARE(source_id)) {
        w_mie(r_mie() & (~(1 << (source_id & 0x1f))));
    } else {
        source_id -= LOCAL_INTERRUPT_MAX_NUM;
        // PLIC 全局控制器的0号中断为无效中断
        if (source_id == LOCAL_INTERRUPT_MAX_NUM) {
            return PLIC_ERRNO_SOURCE_ID_IS_ZERO;
        }
        PLIC_CLEAN_IE(PLIC_BASE_ADDR, get_context_id(), source_id);
    }
    return OS_OK;
}

UINT32 osz_interrupt_set_pri(UINT32 source_id, UINT32 pri)
{
    if (source_id >= OSZ_CFG_INT_LIMIT) {
        return PLIC_ERRNO_INVALID_SOURCE_ID;
    }

    if (INT_IS_TIMER(source_id) || INT_IS_SOFTWARE(source_id)) {
        return OS_OK;
    } else {
        source_id -= LOCAL_INTERRUPT_MAX_NUM;
        // PLIC 全局控制器的0号中断为无效中断
        if (source_id == LOCAL_INTERRUPT_MAX_NUM) {
            return PLIC_ERRNO_SOURCE_ID_IS_ZERO;
        }
        PLIC_SET_PRIORITY(PLIC_BASE_ADDR, source_id, pri);
    }
    return OS_OK;
}

UINT32 osz_interrupt_set_pri_th(UINT32 pri_th)
{
    PLIC_SET_PRIORITY_TH(PLIC_BASE_ADDR, get_context_id(), pri_th);
    return OS_OK;
}