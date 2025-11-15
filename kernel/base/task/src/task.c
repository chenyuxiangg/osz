#include "comm.h"
#include "pri_queue.h"
#include "mem.h"
#include "task.h"
#include "schedule.h"
#include "inner_task_err.h"
#include "timer.h"
#include "barrier.h"

#define US_PER_MS       (1000)

STATIC uint8_t SECTION_KERNEL_INIT_DATA g_idle_task_stack[OSZ_CFG_TASK_STACK_DEFAULT_SIZE];
STATIC uint8_t SECTION_KERNEL_INIT_DATA g_app_task_stack[OSZ_CFG_TASK_STACK_DEFAULT_SIZE];
STATIC osz_list_t SECTION_KERNEL_INIT_DATA g_freelist;
STATIC osz_list_t SECTION_KERNEL_INIT_DATA g_pendlist;
STATIC uint32_t     SECTION_KERNEL_INIT_DATA g_task_sortlink_bitmap;
STATIC osz_sortlink_t  SECTION_KERNEL_INIT_DATA g_task_sortlink;
osz_task_t         *gp_new_task __attribute__((section(".data")));
osz_task_t         *gp_current_task __attribute__((section(".data")));
osz_task_t SECTION_KERNEL_INIT_DATA g_tasks[OSZ_CFG_TASK_LIMIT] = { 0 };

/*
* description:
*   当任务第一次进入时，a0会保存task_id的值，因此会直接调用任务所对应的
*   回调函数；当任务主动调度时，mepc会被置为ra，详见save_current_task
*/
STATIC void_t inner_task_handler(uint16_t task_id)
{
    g_tasks[task_id].tsk_entry(g_tasks[task_id].data);
}

STATIC void_t inner_task_exit()
{
    uint16_t task_id = osz_get_current_tid();
    osz_delete_task(task_id);
}

STATIC void_t idle_task_thread(void_t *data)
{
    while(1) {
        wfi();
    }
}

STATIC SECTION_KERNEL_INIT_TEXT uint32_t inner_task_module_init(void_t)
{
    if (OSZ_CFG_TASK_LIMIT <= 0) {
        return TASK_INIT_TASK_MAX_NUM_INVALID_ERR;
    }

    dlink_init(&g_freelist);
    dlink_init(&g_pendlist);
    sortlink_init(&g_task_sortlink);

    for (uint32_t i = 0; i < OSZ_CFG_TASK_LIMIT; ++i) {
        // init task state
        g_tasks[i].tsk_state = (int8_t)TSK_FLAG_FREE;
        // init task list
        dlink_insert_tail(&g_freelist, (TASK_LIST(i, free)));
    }
    return OS_OK;
}
MODULE_INIT(inner_task_module_init, l1)

STATIC uint32_t inner_task_insert_sortlink(osz_sortlink_t *link)
{
    g_task_sortlink_bitmap |= (1 << osz_get_current_tid());
    return sortlink_insert(&g_task_sortlink, link);
}

STATIC bool_t inner_task_sortlink_is_mounted(uint32_t task_id)
{
    return ((g_task_sortlink_bitmap >> task_id) & 0x1);
}

STATIC bool_t inner_task_pending_is_mounted(uint32_t task_id)
{
    if (task_id >= OSZ_CFG_TASK_LIMIT) {
        return FALSE;
    }
    return ((TASK_STATE(task_id) & TSK_FLAG_PENDING) == TSK_FLAG_PENDING);
}

STATIC uint32_t inner_task_delete_sortlink(osz_sortlink_t *link, uint16_t task_id)
{
    g_task_sortlink_bitmap &= ~(1 << task_id);
    return sortlink_delete(link);
}

STATIC uint32_t inner_check_params(osz_task_params_t *params)
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

STATIC osz_task_t *inner_get_free_task_cb(void_t)
{
    osz_task_t *next_cb = NULL;
    if (DLINK_EMPTY(&g_freelist)) {
        return next_cb;
    }
    uint32_t intSave = 0;
    TASK_INT_LOCK(&intSave);
    osz_list_t *next_free_node = dlink_del_node(g_freelist.next);
    TASK_INT_UNLOCK(intSave);
    next_cb = STRUCT_ENTRY(osz_task_t, tsk_list_free, next_free_node);
    return next_cb;
}

STATIC void_t inner_new_task_fill_context(uint32_t task_id)
{
    // need to config mepc、mstatus、sp
    osz_task_context_t *context = (osz_task_context_t *)(g_tasks[task_id].tsk_stack_top - sizeof(osz_task_context_t));
    ((uint32_t *)context)[0] = TASK_CONTEXT_INIT_BASE_VALUE;
    for (int i = 1; i < sizeof(osz_task_context_t)/sizeof(uint32_t); ++i) {
        ((uint32_t *)context)[i] = ((uint32_t *)context)[i-1] + TASK_CONTEXT_INIT_STEP;
    }
    context->mepc = (uint32_t)inner_task_handler;
    context->mstatus = MSTATUS_MPP | MSTATUS_MPIE;  // must be set 3 to mpp，otherwith mret will be enter trap
    context->a0 = task_id;
    context->ra = (uint32_t)inner_task_exit;
    g_tasks[task_id].tsk_context_pointer = (uintptr_t)context;
}

STATIC void_t inner_new_task_fill_magic_for_stack(osz_task_params_t *params)
{
    // must be keep stack_size align to 16
    for (uint32_t i = 0; i < params->stack_size/sizeof(uint32_t); ++i) {
        ((uint32_t *)params->stack_base)[i] = TASK_STACK_INIT_MAGIC;
    }
}

STATIC void_t inner_new_task_stack_init(uint16_t task_id, osz_task_params_t *params)
{
    /* osz_create_task has checkd params */
    g_tasks[task_id].tsk_stack_top = params->stack_base + params->stack_size;

    g_tasks[task_id].tsk_stack_size = params->stack_size;
    inner_new_task_fill_magic_for_stack(params);
    inner_new_task_fill_context(task_id);
}

STATIC void_t inner_new_task_init(uint16_t task_id, osz_task_params_t *params)
{
    /* osz_create_task has checkd params */
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

STATIC void_t inner_del_task_on_ready(uint16_t task_id)
{
    // need to dequeue
    pri_queue_dequeue(g_tasks[task_id].tsk_priority, (TASK_LIST(task_id, ready)));

    // change task state
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_READY);
    g_need_preemp = FALSE;
}

STATIC void_t inner_del_task_on_running(uint16_t task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
    g_need_preemp = TRUE;
}

STATIC void_t inner_del_task_on_pending(uint16_t task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_PENDING);
    g_need_preemp = FALSE;
}

STATIC void_t inner_del_task_on_blocking(uint16_t task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_BLOCKING);
    g_need_preemp = FALSE;
}

STATIC void_t inner_task_ready2pend(uint16_t task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_READY);
    pri_queue_dequeue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready));
}

STATIC void_t inner_task_run2pend(uint16_t task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
}

STATIC void_t inner_task_block2pend(uint16_t task_id)
{
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_BLOCKING);
    osz_sortlink_t *sl = STRUCT_ENTRY(osz_sortlink_t, list, TASK_LIST(task_id, blocking));
    inner_task_delete_sortlink(sl, task_id);
}

STATIC void_t inner_task_check_timeout(void_t)
{
    uint32_t intSave = 0;
    TASK_INT_LOCK(&intSave);
    osz_sortlink_t *sl = STRUCT_ENTRY(osz_sortlink_t, list, SORTLINK_LIST(g_task_sortlink).next);
    if (PSORTLINK_TIMEOUT(sl) != 0) {
        PSORTLINK_TIMEOUT(sl) -= 1;
    } else {
        do {
            osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_blocking, sl);
            uint16_t task_id = osz_get_task_id_by_task_cb(task);
            TASK_STATUS_CLEAN(task_id, TSK_FLAG_BLOCKING);
            TASK_STATUS_SET(task_id, TSK_FLAG_READY);
            inner_task_delete_sortlink(sl, task_id);
            if (inner_task_pending_is_mounted(task_id)) {
                dlink_del_node(TASK_LIST(task_id, pending));
            }
            pri_queue_enqueue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready), EQ_MODE_TAIL);
            sl = STRUCT_ENTRY(osz_sortlink_t, list, SORTLINK_LIST(g_task_sortlink).next);
        } while((sl != NULL) && (PSORTLINK_TIMEOUT(sl) == 0));
        g_need_preemp = TRUE;
    }
    TASK_INT_UNLOCK(intSave);
}

STATIC void_t inner_task_check_preemp(void_t)
{
    uint32_t intSave = 0;
    TASK_INT_LOCK(&intSave);
    g_need_preemp = (pri_queue_get_bitmap_low_bit() != OS_NOK) ? TRUE : FALSE;
    TASK_INT_UNLOCK(intSave);
}

/* ======== external interface ======== */

uint32_t osz_create_task(uint16_t *task_id, osz_task_params_t *params)
{
    uint32_t check_ret = OS_OK;
    check_ret = inner_check_params(params);
    if (check_ret != OS_OK) {
        return check_ret;
    }

    osz_task_t *free_task_cb = inner_get_free_task_cb();
    if (free_task_cb == NULL) {
        return TASK_CREATE_TASK_OVER_LIMIT_ERR;
    }

    *task_id = TASK_GET_TASKID_BY_TASKCB(free_task_cb);
    inner_new_task_init(*task_id, params);
    return OS_OK;
}

void_t osz_delete_task(uint32_t task_id)
{
    uint32_t intSave = 0;
    TASK_INT_LOCK(&intSave);
    if (task_id >= OSZ_CFG_TASK_LIMIT) {
        return;
    }
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
    // mount task_id to free list
    TASK_STATE(task_id) |= TSK_FLAG_FREE;
    dlink_insert_tail(&g_freelist, (TASK_LIST(task_id, free)));

    if (g_need_preemp == TRUE) {
        os_schedule();
    }
    TASK_INT_UNLOCK(intSave);
}

uint32_t osz_create_idle_task(uint16_t *task_id)
{
    osz_task_params_t params = {
        .name = "idle",
        .stack_attr = STACK_MEM_STATIC,
        .stack_base = (uintptr_t)&g_idle_task_stack,
        .stack_size = OSZ_CFG_TASK_STACK_DEFAULT_SIZE,
        .priority = TASK_PRIORITY_DEFAULT,
        .thread = (task_callback_t)idle_task_thread,
        .data = NULL
    };
    uint32_t ret = OS_OK;
    ret = osz_create_task(task_id, &params);
    if (ret != OS_OK) {
        return ret;
    }
    osz_task_resume(*task_id);
    return ret;
}

uint32_t osz_create_app_task(uint16_t *task_id)
{
    osz_task_params_t params = {
        .name = "app_main",
        .stack_attr = STACK_MEM_STATIC,
        .stack_base = (uintptr_t)&g_app_task_stack,
        .stack_size = OSZ_CFG_TASK_STACK_DEFAULT_SIZE,
        .priority = TASK_PRIORITY_DEFAULT,
        .thread = (task_callback_t)app_main,
        .data = NULL
    };
    uint32_t ret = OS_OK;
    ret = osz_create_task(task_id, &params);
    if (ret != OS_OK) {
        return ret;
    }
    osz_task_resume(*task_id);
    return ret;
}

uint32_t osz_get_current_tid()
{
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    uint32_t id = osz_get_task_id_by_task_cb(gp_current_task);
    ARCH_INT_UNLOCK(intsave);
    return id;
}

osz_task_t *osz_get_taskcb_by_tid(uint32_t tid)
{
    if (tid < 0 || tid >= OSZ_CFG_TASK_LIMIT) {
        return NULL;
    }
    return &g_tasks[tid];
}

uint32_t osz_get_task_priotity_by_tid(uint32_t tid)
{
    return g_tasks[tid].tsk_priority;
}

uint32_t osz_set_task_priotity_by_tid(uint32_t tid, uint32_t new_pri)
{
    uint32_t old_pri = g_tasks[tid].tsk_priority;
    g_tasks[tid].tsk_priority = new_pri;
    return old_pri;
}

uint16_t osz_get_task_id_by_task_cb(osz_task_t *tcb)
{
    ASSERT(tcb);
    return (((uint8_t *)tcb - (uint8_t *)&g_tasks[0]) / sizeof(osz_task_t));
}

/*
* This function dont release CPU.
*/
void_t osz_udelay(uint64_t usec)
{
    uint64_t tick = usec / OSZ_CFG_TASK_TICK_PER_US;
    uint64_t pre_time_stamp = 0;
    uint64_t cur_time_stamp = 0;
    drv_timer_get_time_stamp(&pre_time_stamp);
    do {
        drv_timer_get_time_stamp(&cur_time_stamp);
    } while(tick > (cur_time_stamp - pre_time_stamp));
}

/*
* This function will release CPU.
*/
void_t osz_msleep(uint64_t msec)
{
    uint32_t intSave = 0;
    uint64_t tick = ((msec * US_PER_MS) * CYCLES_PER_US) / CYCLES_PER_TICK;
    uint16_t task_id = osz_get_current_tid();
    TASK_INT_LOCK(&intSave);
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(task_id, TSK_FLAG_BLOCKING);
    osz_sortlink_t *sl = STRUCT_ENTRY(osz_sortlink_t, list, TASK_LIST(task_id, blocking));
    if (inner_task_sortlink_is_mounted(task_id)) {
        inner_task_delete_sortlink(sl, task_id);
    }
    PSORTLINK_TIMEOUT(sl) = tick;
    inner_task_insert_sortlink(sl);
    TASK_INT_UNLOCK(intSave);
    os_schedule();
}

void_t osz_update_task(void_t)
{
    if (!DLINK_EMPTY(&SORTLINK_LIST(g_task_sortlink))) {
        inner_task_check_timeout();
    }
    inner_task_check_preemp();
}

void_t osz_task_suspend(uint16_t task_id)
{
    uint32_t intSave = 0;
    TASK_INT_LOCK(&intSave);
    if (TASK_STATE(task_id) == TSK_FLAG_PENDING || TASK_STATE(task_id) == TSK_FLAG_FREE) {
        TASK_INT_UNLOCK(intSave);
        return;
    }
    osz_task_flags_t status = TASK_STATE(task_id);
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

void_t osz_task_resume(uint16_t task_id)
{
    uint32_t intSave = 0;
    TASK_INT_LOCK(&intSave);
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_PENDING);
    TASK_STATUS_SET(task_id, TSK_FLAG_READY);
    dlink_del_node(TASK_LIST(task_id, pending));
    pri_queue_enqueue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready), EQ_MODE_HEAD);
    TASK_INT_UNLOCK(intSave);
}

void_t osz_task_yeild()
{
    uint32_t intSave = 0;
    uint16_t task_id = osz_get_current_tid();
    if (TASK_STATE(task_id) != TSK_FLAG_RUNNING) {
        return;
    }
    TASK_INT_LOCK(&intSave);
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(task_id, TSK_FLAG_READY);
    pri_queue_enqueue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready), EQ_MODE_TAIL);
    TASK_INT_UNLOCK(intSave);
}

uint32_t osz_task_wait(uint64_t timeout)
{
    if (timeout == 0) {
        return TASK_NO_WAIT_ERR;
    }
    uint16_t tid = osz_get_current_tid();
    uint32_t intsave = 0;
    if (timeout == WAIT_FOREVER) {
        os_schedule();
        return TASK_WAIT_WAKE_UP_ERR;
    }
    osz_msleep(timeout);
    return TASK_WAIT_TIMEOUT_ERR;
}

uint32_t osz_task_wake(uint16_t task_id)
{
    if (task_id < 0 || task_id >= OSZ_CFG_TASK_LIMIT) {
        return TASK_WAKE_TASK_ID_NOT_EXIST;
    }
    
    uint32_t intSave = 0;
    TASK_INT_LOCK(&intSave);
    if ((TASK_STATE(task_id) & TSK_FLAG_BLOCKING) == TSK_FLAG_BLOCKING) {
        TASK_STATUS_CLEAN(task_id, TSK_FLAG_BLOCKING);
        osz_sortlink_t *sl = STRUCT_ENTRY(osz_sortlink_t, list, TASK_LIST(task_id, blocking));
        inner_task_delete_sortlink(sl, task_id);
    }
    if ((TASK_STATE(task_id) & TSK_FLAG_PENDING) == TSK_FLAG_PENDING) {
        TASK_STATUS_CLEAN(task_id, TSK_FLAG_PENDING);
        dlink_del_node(TASK_LIST(task_id, pending));
    }
    TASK_STATUS_SET(task_id, TSK_FLAG_READY);
    pri_queue_enqueue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready), EQ_MODE_TAIL);
    TASK_INT_UNLOCK(intSave);
    return OS_OK;
}

void_t switch_hook(void_t)
{
    return;
}