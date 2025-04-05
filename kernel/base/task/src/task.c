#include "comm.h"
#include "pri_queue.h"
#include "mem.h"
#include "task.h"
#include "schedule.h"
#include "inner_task_err.h"
#include "timer.h"
#include "barrier.h"

#define US_PER_MS       (1000)

STATIC UINT8 SECTION_KERNEL_INIT_DATA g_idle_task_stack[OSZ_CFG_TASK_STACK_DEFAULT_SIZE];
STATIC DLINK_NODE SECTION_KERNEL_INIT_DATA g_freelist;
STATIC DLINK_NODE SECTION_KERNEL_INIT_DATA g_pendlist;
STATIC UINT32   g_task_sortlink_bitmap;
TASK_CB         *gp_new_task __attribute__((section(".data")));
TASK_CB         *gp_current_task __attribute__((section(".data")));
TASK_CB SECTION_KERNEL_INIT_DATA g_tasks[OSZ_CFG_TASK_LIMIT] = { 0 };

STATIC VOID inner_task_handler(UINT16 task_id)
{
    if (task_id < 0 || task_id > OSZ_CFG_TASK_LIMIT) {
        return;
    }
    g_tasks[task_id].tsk_entry(g_tasks[task_id].data);
}

STATIC VOID inner_task_exit(UINT16 task_id)
{
    os_delete_task(task_id);
}

STATIC VOID idle_task_thread(VOID *data)
{
    while(1) {
        wfi();
    }
}

STATIC SECTION_KERNEL_INIT_TEXT UINT32 inner_task_module_init(VOID)
{
    if (OSZ_CFG_TASK_LIMIT <= 0) {
        return TASK_INIT_TASK_MAX_NUM_INVALID_ERR;
    }

    dlink_init(&g_freelist);
    dlink_init(&g_pendlist);
    sortlink_init(&g_task_sortlink);
    g_task_sortlink_bitmap = 0;

    for (UINT32 i = 0; i < OSZ_CFG_TASK_LIMIT; ++i) {
        // init task state
        g_tasks[i].tsk_state = (CHAR)TSK_FLAG_FREE;
        // init task list
        dlink_insert_tail(&g_freelist, (TASK_LIST(i, free)));
    }
    return OS_OK;
}
MODULE_INIT(inner_task_module_init, l1)

STATIC UINT32 inner_check_params(TASK_PARAMS *params)
{
    ASSERT(params);
    if (params->name == NULL) {
        return TASK_CREATE_CHECK_NAME_ERR;
    }
    if (params->stack_base == NULL) {
        return TASK_CREATE_CHECK_STACK_BASE_ERR;
    }
    if (params->stack_size <= 0) {
        return TASK_CREATE_CHECK_STACK_SIZE_ERR;
    }
    if (params->thread == NULL) {
        return TASK_CREATE_CHECK_TASK_ENTRY_ERR;
    }
    return OS_OK;
}

STATIC TASK_CB *inner_get_free_task_cb(VOID)
{
    TASK_CB *next_cb = NULL;
    if (DLINK_EMPTY(&g_freelist)) {
        return next_cb;
    }
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    DLINK_NODE *next_free_node = dlink_del_node(g_freelist.next);
    TASK_INT_UNLOCK(intSave);
    next_cb = STRUCT_ENTRY(TASK_CB, tsk_list_free, next_free_node);
    return next_cb;
}

STATIC VOID inner_new_task_fill_context(UINT32 task_id)
{
    // need to config mepc、mstatus、sp
    TASK_CONTEXT *context = (TASK_CONTEXT *)(g_tasks[task_id].tsk_stack_top - sizeof(TASK_CONTEXT));
    ((UINT32 *)context)[0] = TASK_CONTEXT_INIT_BASE_VALUE;
    for (int i = 1; i < sizeof(TASK_CONTEXT)/sizeof(UINT32); ++i) {
        ((UINT32 *)context)[i] = ((UINT32 *)context)[i-1] + TASK_CONTEXT_INIT_STEP;
    }
    context->mepc = (UINT32)inner_task_handler;
    context->mstatus = MSTATUS_MPP | MSTATUS_MPIE;  // must be set 3 to mpp，otherwith mret will be enter trap
    context->a0 = task_id;
    context->ra = (UINT32)inner_task_exit;
    g_tasks[task_id].tsk_context_pointer = (UINTPTR)context;
}

STATIC VOID inner_new_task_fill_magic_for_stack(TASK_PARAMS *params)
{
    // must be keep stack_size align to 16
    for (UINT32 i = 0; i < params->stack_size/sizeof(UINT32); ++i) {
        ((UINT32 *)params->stack_base)[i] = TASK_STACK_INIT_MAGIC;
    }
}

STATIC VOID inner_new_task_stack_init(UINT16 task_id, TASK_PARAMS *params)
{
    /* os_create_task has checkd params */
    g_tasks[task_id].tsk_stack_top = params->stack_base + params->stack_size;

    g_tasks[task_id].tsk_stack_size = params->stack_size;
    inner_new_task_fill_magic_for_stack(params);
    inner_new_task_fill_context(task_id);
}

STATIC VOID inner_new_task_init(UINT16 task_id, TASK_PARAMS *params)
{
    /* os_create_task has checkd params */
    TASK_NAME(task_id) = params->name;
    TASK_THREAD(task_id) = params->thread;
    TASK_DATA(task_id) = params->data;
    TASK_STACK_ATTR(task_id) = params->stack_attr;
    TASK_PRIORITY(task_id) = params->priority;
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_FREE);
    TASK_STATUS_SET(task_id, TSK_FLAG_PENDING);
    dlink_insert_tail(&g_pendlist, TASK_LIST(task_id, pending));
    return inner_new_task_stack_init(task_id, params);
}

STATIC VOID inner_del_task_on_ready(UINT16 task_id)
{
    // need to dequeue
    pri_queue_dequeue(g_tasks[task_id].tsk_priority, (TASK_LIST(task_id, ready)));

    // change task state
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_READY);
    g_need_preemp = FALSE;
}

STATIC VOID inner_del_task_on_running(UINT16 task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
    g_need_preemp = TRUE;
}

STATIC VOID inner_del_task_on_pending(UINT16 task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_PENDING);
    g_need_preemp = FALSE;
}

STATIC VOID inner_del_task_on_blocking(UINT16 task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_BLOCKING);
    g_need_preemp = FALSE;
}

STATIC BOOL inner_task_sortlink_is_mounted(UINT32 task_id)
{
    return ((g_task_sortlink_bitmap >> task_id) & 0x1);
}

STATIC VOID inner_task_ready2pend(UINT16 task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_READY);
    pri_queue_dequeue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready));
}

STATIC VOID inner_task_run2pend(UINT16 task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
}

STATIC VOID inner_task_block2pend(UINT16 task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_BLOCKING);
    SORT_LINK *sl = STRUCT_ENTRY(SORT_LINK, list, TASK_LIST(task_id, blocking));
    sortlink_delete(sl);
}

STATIC VOID inner_task_check_timeout(VOID)
{
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    SORT_LINK *sl = STRUCT_ENTRY(SORT_LINK, list, SORTLINK_LIST(g_task_sortlink).next);
    if (PSORTLINK_TIMEOUT(sl) != 0) {
        PSORTLINK_TIMEOUT(sl) -= 1;
    } else {
        TASK_CB *task = STRUCT_ENTRY(TASK_CB, tsk_list_blocking, sl);
        UINT16 task_id = os_get_task_id_by_task_cb(task);
        TASK_STATUS_CLEAN(task_id, TSK_FLAG_BLOCKING);
        TASK_STATUS_SET(task_id, TSK_FLAG_READY);
        sortlink_delete(sl);
        pri_queue_enqueue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready), EQ_MODE_TAIL);
        g_need_preemp = TRUE;
    }
    TASK_INT_UNLOCK(intSave);
}

STATIC VOID inner_task_check_preemp(VOID)
{
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    g_need_preemp = (pri_queue_get_bitmap_low_bit() != OS_NOK) ? TRUE : FALSE;
    TASK_INT_UNLOCK(intSave);
}

/* ======== external interface ======== */

UINT32 os_create_task(UINT16 *task_id, TASK_PARAMS *params)
{
    UINT32 check_ret = OS_OK;
    check_ret = inner_check_params(params);
    if (check_ret != OS_OK) {
        return check_ret;
    }

    TASK_CB *free_task_cb = inner_get_free_task_cb();
    if (free_task_cb == NULL) {
        return TASK_CREATE_TASK_OVER_LIMIT_ERR;
    }

    *task_id = TASK_GET_TASKID_BY_TASKCB(free_task_cb);
    inner_new_task_init(*task_id, params);
    return OS_OK;
}

VOID os_delete_task(UINT32 task_id)
{
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    if (TASK_STATE(task_id) == TSK_FLAG_FREE) {
        return;
    }
    switch (TASK_STATE(task_id))
    {
        case TSK_FLAG_RUNNING:
        {
            inner_del_task_on_running(task_id);
            break;
        }
        case TSK_FLAG_READY:
        {
            inner_del_task_on_ready(task_id);
            break;
        }
        case TSK_FLAG_PENDING:
        {
            inner_del_task_on_pending(task_id);
            break;
        }
        case TSK_FLAG_BLOCKING:
        {
            inner_del_task_on_blocking(task_id);
            break;
        }
        default:
            break;
    }
    // free stack mem
    if (TASK_STACK_ATTR(task_id) == STACK_MEM_DYNAMIC) {
        osz_free((void *)(g_tasks[task_id].tsk_stack_top - g_tasks[task_id].tsk_stack_size));
    }
    // free task param
    if (TASK_DATA(task_id) != NULL) {
        osz_free(g_tasks[task_id].data);
        g_tasks[task_id].data = NULL;
    }
    // mount task_id to free list
    dlink_insert_tail(&g_freelist, (TASK_LIST(task_id, free)));
    TASK_STATE(task_id) |= TSK_FLAG_FREE;

    if (g_need_preemp == TRUE) {
        os_schedule();
    }
    TASK_INT_UNLOCK(intSave);
}

UINT32 task_create_idle_task(UINT16 *task_id)
{
    TASK_PARAMS params = {
        .name = "idle",
        .stack_attr = STACK_MEM_STATIC,
        .stack_base = (UINTPTR)&g_idle_task_stack,
        .stack_size = OSZ_CFG_TASK_STACK_DEFAULT_SIZE,
        .priority = TASK_PRIORITY_DEFAULT,
        .thread = (TASK_THREAD_TYPE)idle_task_thread,
        .data = NULL
    };
    UINT32 ret = OS_OK;
    ret = os_create_task(task_id, &params);
    if (ret != OS_OK) {
        return ret;
    }
    os_task_resume(*task_id);
    return ret;
}

UINT32 os_get_current_tid()
{
    return os_get_task_id_by_task_cb(gp_current_task);
}

TASK_CB *os_get_taskcb_by_tid(UINT32 tid)
{
    if (tid < 0 || tid >= OSZ_CFG_TASK_LIMIT) {
        return NULL;
    }
    return &g_tasks[tid];
}

UINT16 os_get_task_id_by_task_cb(TASK_CB *tcb)
{
    ASSERT(tcb);
    return (((UCHAR *)tcb - (UCHAR *)&g_tasks[0]) / sizeof(TASK_CB));
}

/*
* This function dont release CPU.
*/
VOID os_udelay(UINT64 usec)
{
    UINT64 tick = usec / OSZ_CFG_TASK_TICK_PER_US;
    UINT64 pre_time_stamp = 0;
    UINT64 cur_time_stamp = 0;
    drv_timer_get_time_stamp(&pre_time_stamp);
    do {
        drv_timer_get_time_stamp(&cur_time_stamp);
    } while(tick > (cur_time_stamp - pre_time_stamp));
}

/*
* This function will release CPU.
*/
VOID os_msleep(UINT64 msec)
{
    UINT32 intSave = 0;
    UINT64 tick = ((msec * US_PER_MS) / CYCLES_PER_US) / CYCLES_PER_TICK;
    UINT16 task_id = os_get_current_tid();
    TASK_INT_LOCK(&intSave);
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(task_id, TSK_FLAG_BLOCKING);
    SORT_LINK *sl = STRUCT_ENTRY(SORT_LINK, list, TASK_LIST(task_id, blocking));
    if (inner_task_sortlink_is_mounted(task_id)) {
        sortlink_delete(sl);
    }
    PSORTLINK_TIMEOUT(sl) = tick;
    sortlink_insert(sl);
    TASK_INT_UNLOCK(intSave);
    os_schedule();
}

VOID os_update_task(VOID)
{
    if (!DLINK_EMPTY(&SORTLINK_LIST(g_task_sortlink))) {
        inner_task_check_timeout();
    }
    inner_task_check_preemp();
}

VOID os_task_suspend(UINT16 task_id)
{
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    if (TASK_STATE(task_id) == TSK_FLAG_PENDING || TASK_STATE(task_id) == TSK_FLAG_FREE) {
        TASK_INT_UNLOCK(intSave);
        return;
    }
    TASK_FLAGS status = TASK_STATE(task_id);
    switch(status) {
        case TSK_FLAG_READY:
            inner_task_ready2pend(task_id);
            break;
        case TSK_FLAG_RUNNING:
            inner_task_run2pend(task_id);
            break;
        case TSK_FLAG_BLOCKING:
            inner_task_block2pend(task_id);
            break;
        default:
            break;
    }
    TASK_STATUS_SET(task_id, TSK_FLAG_PENDING);
    dlink_insert_tail(&g_pendlist, TASK_LIST(task_id, pending));
    TASK_INT_UNLOCK(intSave);
    if (status == TSK_FLAG_RUNNING) {
        os_schedule();
    }
}

VOID os_task_resume(UINT16 task_id)
{
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_PENDING);
    TASK_STATUS_SET(task_id, TSK_FLAG_READY);
    dlink_del_node(TASK_LIST(task_id, pending));
    pri_queue_enqueue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready), EQ_MODE_HEAD);
    TASK_INT_UNLOCK(intSave);
}

VOID os_task_yeild()
{
    UINT32 intSave = 0;
    UINT16 task_id = os_get_current_tid();
    if (TASK_STATE(task_id) != TSK_FLAG_RUNNING) {
        return;
    }
    TASK_INT_LOCK(&intSave);
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(task_id, TSK_FLAG_READY);
    pri_queue_enqueue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready), EQ_MODE_TAIL);
    TASK_INT_UNLOCK(intSave);
}