#include "softtimer.h"
#include "inner_softtimer_err.h"
#include "timer.h"
#include "sortlink.h"
#include "arch_adapter.h"

#define SOFTTIMER_INVALID 0xFFFFFFFF

STATIC SOFTTIMER_CB g_softtimers[OSZ_CFG_SWT_MAX];
STATIC SORT_LINK g_swt_sortlink;

STATIC VOID softtimer_init(VOID)
{
    sortlink_init(&g_swt_sortlink);
    for (UINT32 i = 0; i < OSZ_CFG_SWT_MAX; ++i) {
        g_softtimers[i].link.list.next = NULL;
        g_softtimers[i].link.list.pre = NULL;
        g_softtimers[i].name = NULL;
        g_softtimers[i].func = NULL;
        g_softtimers[i].args = NULL;
        g_softtimers[i].link.timeout = 0;
        g_softtimers[i].load = 0;
        g_softtimers[i].flags = 0;
    }
}
MODULE_INIT(softtimer_init, l3);

STATIC UINT32 inner_softtimer_check_params(SOFTTIMER_PARAMS *params)
{
    UINT32 ret = OS_OK;
    if (params == NULL) {
        return SOFTTIMER_CHECK_PARAMS_NULL_ERR;
    }
    if (params->func == NULL) {
        return SOFTTIMER_CHECK_FUNC_ERR;
    }
    if (params->timeout <= 0) {
        return SOFTTIMER_CHECK_TIMEOUT_ERR;
    }
    if (params->name == NULL) {
        return SOFTTIMER_CHECK_NAME_ERR;
    }
    if ((params->flags & (SOFTTIMER_FLAG_REPEAT | SOFTTIMER_FLAG_ONESHOT)) == (SOFTTIMER_FLAG_REPEAT | SOFTTIMER_FLAG_ONESHOT)) {
        return SOFTTIMER_CHECK_TIMER_ATTR_ERR;
    }
    return ret;
}

STATIC UINT32 inner_softtimer_get_free_node(VOID)
{
    for (UINT32 i = 0; i < OSZ_CFG_SWT_MAX; i++) {
        if ((g_softtimers[i].flags & SOFTTIMER_FLAG_USED) == 0) {
            return i;
        }
    }
    return SOFTTIMER_INVALID;
}

STATIC VOID inner_softtimer_init(UINT32 swt_id, SOFTTIMER_PARAMS *params)
{
    g_softtimers[swt_id].flags = SOFTTIMER_FLAG_USED | params->flags;
    g_softtimers[swt_id].link.timeout = params->timeout;
    g_softtimers[swt_id].load = params->timeout;
    g_softtimers[swt_id].func = params->func;
    g_softtimers[swt_id].name = params->name;
    g_softtimers[swt_id].args = params->args;
}

STATIC VOID inner_swtmr_check_timeout(VOID)
{
    UINT32 intSave = 0;
    ARCH_INT_LOCK(intSave);
    SORT_LINK *sl = STRUCT_ENTRY(SORT_LINK, list, SORTLINK_LIST(g_swt_sortlink).next);
    if (PSORTLINK_TIMEOUT(sl) != 0) {
        PSORTLINK_TIMEOUT(sl) -= 1;
    } else {
        do {
            SOFTTIMER_CB *swtmr = STRUCT_ENTRY(SOFTTIMER_CB, link, sl);
            if (swtmr->func != NULL) {
                swtmr->func(swtmr->args);
            }
            sortlink_delete(sl);
            PSORTLINK_TIMEOUT(sl) = swtmr->load;
            sortlink_insert(&g_swt_sortlink, sl);
            sl = STRUCT_ENTRY(SORT_LINK, list, SORTLINK_LIST(g_swt_sortlink).next);
        } while((sl != NULL) && (PSORTLINK_TIMEOUT(sl) == 0));
    }
    ARCH_INT_UNLOCK(intSave);
}

UINT32 softtimer_create(UINT32 *swt_id, SOFTTIMER_PARAMS *params)
{
    UINT32 ret = OS_OK;
    UINT32 intSave = 0;
    ret = inner_softtimer_check_params(params);
    if (ret != OS_OK) {
        return ret;
    }
    ARCH_INT_LOCK(intSave);
    *swt_id = inner_softtimer_get_free_node();
    if (*swt_id == SOFTTIMER_INVALID) {
        ret = SOFTTIMER_CHECK_TIMER_INVALID_ERR;
        return ret;
    }
    inner_softtimer_init(*swt_id, params);
    ARCH_INT_UNLOCK(intSave);
    return ret;
}

UINT32 softtimer_delete(UINT32 swt_id)
{
    UINT32 ret = OS_OK;
    UINT32 intSave = 0;
    if (swt_id >= OSZ_CFG_SWT_MAX) {
        return SOFTTIMER_CHECK_TIMER_INVALID_ERR;
    }
    if ((g_softtimers[swt_id].flags & SOFTTIMER_FLAG_USED) != SOFTTIMER_FLAG_USED) {
        return SOFTTIMER_DEL_SWT_NOT_USED_ERR;
    }
    ARCH_INT_LOCK(intSave);
    g_softtimers[swt_id].flags = 0;
    g_softtimers[swt_id].func = NULL;
    g_softtimers[swt_id].args = NULL;
    g_softtimers[swt_id].name = NULL;
    g_softtimers[swt_id].link.timeout = 0;
    g_softtimers[swt_id].load = 0;
    sortlink_delete(&(g_softtimers[swt_id].link));
    ARCH_INT_UNLOCK(intSave);
    return ret;
}

VOID softtimer_start(UINT32 swt_id)
{
    UINT32 intSave = 0;
    if (swt_id >= OSZ_CFG_SWT_MAX) {
        return;
    }
    ARCH_INT_LOCK(intSave);
    sortlink_insert(&g_swt_sortlink, &g_softtimers[swt_id].link);
    ARCH_INT_UNLOCK(intSave);
}

VOID softtimer_stop(UINT32 swt_id)
{
    UINT32 intSave = 0;
    if (swt_id >= OSZ_CFG_SWT_MAX) {
        return;
    }
    ARCH_INT_LOCK(intSave);
    sortlink_delete(&(g_softtimers[swt_id].link));
    ARCH_INT_UNLOCK(intSave);
}

VOID softtimer_ctrl(UINT32 swt_id, SOFTTIMER_CMD cmd, VOID *arg)
{
    UINT32 intSave = 0;
    if (swt_id >= OSZ_CFG_SWT_MAX) {
        return;
    }
    ARCH_INT_LOCK(intSave);
    switch (cmd) {
        case SOFTTIMER_CMD_SET_TIME:
            softtimer_stop(swt_id);
            g_softtimers[swt_id].load = *((UINT32 *)arg);
            softtimer_start(swt_id);
            break;
        case SOFTTIMER_CMD_GET_TIME:
            *((UINT32 *)arg) = g_softtimers[swt_id].link.timeout;
            break;
        case SOFTTIMER_CMD_SET_ONESHOT:
            g_softtimers[swt_id].flags |= SOFTTIMER_FLAG_ONESHOT;
            g_softtimers[swt_id].flags &= ~SOFTTIMER_FLAG_REPEAT;
            break;
        case SOFTTIMER_CMD_SET_REPEAT:
            g_softtimers[swt_id].flags |= SOFTTIMER_FLAG_REPEAT;
            g_softtimers[swt_id].flags &= ~SOFTTIMER_FLAG_ONESHOT;
            break;
        default:
            break;
    }
    ARCH_INT_UNLOCK(intSave);
}

VOID os_update_swtmr(VOID)
{
    inner_swtmr_check_timeout();
}