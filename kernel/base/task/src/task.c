#include "comm.h"
#include "pri_queue.h"
#include "mem.h"
#include "task.h"
#include "schedule.h"
#include "inner_task_err.h"

STATIC UINT8 SECTION_KERNEL_INIT_DATA g_idle_task_stack[OSZ_CFG_TASK_STACK_DEFAULT_SIZE];
STATIC DLINK_NODE SECTION_KERNEL_INIT_DATA g_freelist;
STATIC TASK_SORT_LINK SECTION_KERNEL_INIT_DATA g_sortlink;
STATIC UINT8    g_need_schedule = FALSE;
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
    UINT32 mstatus_v = CSR_READ(mstatus);
    UINT32 mie_v = CSR_READ(mie);
    printf("mstatus: %#x\n", mstatus_v);
    printf("mie: %#x\n", mie_v);
    printf("%s, %d\n", __FUNCTION__, __LINE__);
    while(1);
}

STATIC SECTION_KERNEL_INIT_TEXT VOID innter_task_create_idle_task(VOID)
{
    TASK_PARAMS params = {
        .name = "idle",
        .stack_attr = STACK_MEM_STATIC,
        .stack_base = (UINTPTR)&g_idle_task_stack,
        .stack_size = OSZ_CFG_TASK_STACK_DEFAULT_SIZE,
        .thread = (TASK_THREAD_TYPE)idle_task_thread,
        .data = NULL
    };
    UINT16 task_id;
    UINT32 ret = os_create_task(&task_id, &params);
    if (ret != OS_OK) {
        return;
    }
    TASK_STATE(task_id) &= (~TSK_FLAG_PENDING);
    TASK_STATE(task_id) |= TSK_FLAG_READY;
    gp_new_task = os_get_taskcb_by_tid(task_id);
}
MODULE_INIT(innter_task_create_idle_task, l4)

STATIC SECTION_KERNEL_INIT_TEXT UINT32 inner_task_module_init(VOID)
{
    if (OSZ_CFG_TASK_LIMIT <= 0) {
        return TASK_INIT_TASK_MAX_NUM_INVALID_ERR;
    }

    dlink_init(&g_freelist);
    dlink_init(&(TASK_SORTLINK_LIST(g_sortlink)));
    g_sortlink.timeout = 0xFFFFFFFF;

    for (UINT32 i = 0; i < OSZ_CFG_TASK_LIMIT; ++i) {
        // init task state
        g_tasks[i].tsk_state = (CHAR)TSK_FLAG_FREE;
        // init task list
        dlink_insert_tail(&g_freelist, &(TASK_LIST(i, free)));
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
    // if (!IS_ALIGN(params->stack_base, 16)) {
    //     return TASK_CREATE_CHECK_STACK_AGLIGN_ERR;
    // }
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
    TASK_INT_LOCK();
    DLINK_NODE *next_free_node = dlink_del_node(g_freelist.next);
    TASK_INT_UNLOCK();
    next_cb = DLINK_ENTRY(TASK_CB, tsk_list_free, next_free_node);
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
    TASK_PRIORITY(task_id) = TASK_PRIORITY_DEFAULT;
    TASK_STATE_CLEAN(task_id, TSK_FLAG_FREE);
    TASK_STATE_SET(task_id, TSK_FLAG_PENDING);
    return inner_new_task_stack_init(task_id, params);
}

STATIC VOID inner_del_task_on_ready(UINT16 task_id)
{
    // need to dequeue
    pri_queue_dequeue(g_tasks[task_id].tsk_priority, &(TASK_LIST(task_id, ready)));

    // change task state
    TASK_STATE(task_id) &= ~(TSK_FLAG_READY);
    g_need_schedule = FALSE;
}

STATIC VOID inner_del_task_on_running(UINT16 task_id)
{
    TASK_STATE(task_id) &= ~(TSK_FLAG_RUNNING);
    g_need_schedule = TRUE;
}

STATIC VOID inner_del_task_on_pending(UINT16 task_id)
{
    TASK_STATE(task_id) &= ~(TSK_FLAG_PENDING);
    g_need_schedule = FALSE;
}

STATIC VOID inner_del_task_on_blocking(UINT16 task_id)
{
    TASK_STATE(task_id) &= ~(TSK_FLAG_BLOCKING);
    g_need_schedule = FALSE;
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
    TASK_INT_LOCK();
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
    dlink_insert_tail(&g_freelist, &(TASK_LIST(task_id, free)));
    TASK_STATE(task_id) |= TSK_FLAG_FREE;

    if (g_need_schedule == TRUE) {
        os_schedule();
    }
    TASK_INT_UNLOCK();
}

UINT32 os_get_current_tid()
{
    for (int i = 0; i < OSZ_CFG_TASK_LIMIT; ++i) {
        if (TASK_STATE(i) == TSK_FLAG_RUNNING) {
            return i;
        }
    }
    return -1;
}

TASK_CB *os_get_taskcb_by_tid(UINT32 tid)
{
    if (tid < 0 || tid >= OSZ_CFG_TASK_LIMIT) {
        return NULL;
    }
    return &g_tasks[tid];
}

VOID os_udelay(UINT32 usec)
{
    UINT32 count = 50000000;
    count *= usec;
    while(count--);
}