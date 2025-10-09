#include "softtimer.h"
#include "inner_softtimer_err.h"
#include "timer.h"
#include "task.h"
#include "mem.h"
#include "event.h"
#include "sortlink.h"
#include "arch_adapter.h"

#define SOFTTIMER_INVALID 0xFFFFFFFF

STATIC SOFTTIMER_CB g_softtimers[OSZ_CFG_SWT_MAX];
STATIC SORT_LINK g_swt_sortlink;
STATIC DLINK_NODE g_swt_work_queue;

STATIC SOFTTIMER_WORKER *inner_get_next_swt_woker(void_t)
{
    DLINK_NODE *pos = NULL;
    SOFTTIMER_WORKER *woker = NULL;
    // EVENT_CB ecb = {
    //     .event_op = EVENT_OP_OR | EVENT_OP_CLR,
    //     .event_wait = 
    // }
    // if (!DLINK_EMPTY(&g_swt_work_queue)) {
    //     pos = g_swt_work_queue.next;
    //     woker = STRUCT_ENTRY(SOFTTIMER_WORKER, link, pos);
    // }
    return woker;
}

STATIC void_t inner_swtmr_entry(void_t *data)
{
    while (1) {
        uint32_t intSave = 0;
        ARCH_INT_LOCK(intSave);
        SOFTTIMER_WORKER *woker = inner_get_next_swt_woker();
        ARCH_INT_UNLOCK(intSave);
        if (woker == NULL) {
            continue;
        }
        woker->func(woker->args);
        osz_free(woker);
        woker = NULL;
    }
}

STATIC void_t inner_swtmr_task_create(void_t)
{
    void_t *stack_base = osz_zalloc(OSZ_CFG_SOFTTIMER_TASK_STACK);
    osz_task_params_t task_params = {
        .name = "swtmr",
        .stack_base = (uintptr_t)stack_base,
        .stack_size = OSZ_CFG_SOFTTIMER_TASK_STACK,
        .priority = OSZ_CFG_SOFTTIMER_TASK_PRIORITY,
        .stack_attr = STACK_MEM_DYNAMIC,
        .thread = (task_callback_t)inner_swtmr_entry,
    };
    uint16_t task_id = -1;
    uint32_t ret = osz_create_task(&task_id, &task_params);
    if (ret != OS_OK) {
        printf("ERR: Create swtmr task failed\n");
    }
}

STATIC void_t softtimer_init(void_t)
{
    sortlink_init(&g_swt_sortlink);
    dlink_init(&g_swt_work_queue);
    for (uint32_t i = 0; i < OSZ_CFG_SWT_MAX; ++i) {
        g_softtimers[i].link.list.next = NULL;
        g_softtimers[i].link.list.pre = NULL;
        g_softtimers[i].name = NULL;
        g_softtimers[i].func = NULL;
        g_softtimers[i].args = NULL;
        g_softtimers[i].link.timeout = 0;
        g_softtimers[i].load = 0;
        g_softtimers[i].flags = 0;
    }
    inner_swtmr_task_create();
}
MODULE_INIT(softtimer_init, l3);

STATIC uint32_t inner_softtimer_check_params(SOFTTIMER_PARAMS *params)
{
    uint32_t ret = OS_OK;
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

STATIC uint32_t inner_softtimer_get_free_node(void_t)
{
    for (uint32_t i = 0; i < OSZ_CFG_SWT_MAX; i++) {
        if ((g_softtimers[i].flags & SOFTTIMER_FLAG_USED) == 0) {
            return i;
        }
    }
    return SOFTTIMER_INVALID;
}

STATIC void_t inner_softtimer_init(uint32_t swt_id, SOFTTIMER_PARAMS *params)
{
    g_softtimers[swt_id].flags = SOFTTIMER_FLAG_USED | params->flags;
    g_softtimers[swt_id].link.timeout = params->timeout;
    g_softtimers[swt_id].load = params->timeout;
    g_softtimers[swt_id].func = params->func;
    g_softtimers[swt_id].name = params->name;
    g_softtimers[swt_id].args = params->args;
}

STATIC void_t inner_swtmr_check_timeout(void_t)
{
    uint32_t intSave = 0;
    ARCH_INT_LOCK(intSave);
    SORT_LINK *sl = STRUCT_ENTRY(SORT_LINK, list, SORTLINK_LIST(g_swt_sortlink).next);
    if (PSORTLINK_TIMEOUT(sl) != 0) {
        PSORTLINK_TIMEOUT(sl) -= 1;
    } else {
        do {
            SOFTTIMER_CB *swtmr = STRUCT_ENTRY(SOFTTIMER_CB, link, sl);
            if (swtmr->func != NULL) {
                SOFTTIMER_WORKER *worker = osz_zalloc(sizeof(SOFTTIMER_WORKER));
                worker->id = GET_SWTMR_ID_FROM_CB(swtmr);
                worker->func = swtmr->func;
                worker->args = swtmr->args;
                dlink_insert_tail(&g_swt_work_queue, &(worker->link));
            }
            sortlink_delete(sl);
            PSORTLINK_TIMEOUT(sl) = swtmr->load;
            sortlink_insert(&g_swt_sortlink, sl);
            sl = STRUCT_ENTRY(SORT_LINK, list, SORTLINK_LIST(g_swt_sortlink).next);
        } while((sl != NULL) && (PSORTLINK_TIMEOUT(sl) == 0));
    }
    ARCH_INT_UNLOCK(intSave);
}

uint32_t softtimer_create(uint32_t *swt_id, SOFTTIMER_PARAMS *params)
{
    uint32_t ret = OS_OK;
    uint32_t intSave = 0;
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

uint32_t softtimer_delete(uint32_t swt_id)
{
    uint32_t ret = OS_OK;
    uint32_t intSave = 0;
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

void_t softtimer_start(uint32_t swt_id)
{
    uint32_t intSave = 0;
    if (swt_id >= OSZ_CFG_SWT_MAX) {
        return;
    }
    ARCH_INT_LOCK(intSave);
    sortlink_insert(&g_swt_sortlink, &g_softtimers[swt_id].link);
    ARCH_INT_UNLOCK(intSave);
}

void_t softtimer_stop(uint32_t swt_id)
{
    uint32_t intSave = 0;
    if (swt_id >= OSZ_CFG_SWT_MAX) {
        return;
    }
    ARCH_INT_LOCK(intSave);
    sortlink_delete(&(g_softtimers[swt_id].link));
    ARCH_INT_UNLOCK(intSave);
}

void_t softtimer_ctrl(uint32_t swt_id, SOFTTIMER_CMD cmd, void_t *arg)
{
    uint32_t intSave = 0;
    if (swt_id >= OSZ_CFG_SWT_MAX) {
        return;
    }
    ARCH_INT_LOCK(intSave);
    switch (cmd) {
        case SOFTTIMER_CMD_SET_TIME:
            softtimer_stop(swt_id);
            g_softtimers[swt_id].load = *((uint32_t *)arg);
            softtimer_start(swt_id);
            break;
        case SOFTTIMER_CMD_GET_TIME:
            *((uint32_t *)arg) = g_softtimers[swt_id].link.timeout;
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

void_t os_update_swtmr(void_t)
{
    inner_swtmr_check_timeout();
}