#include "string.h"
#include "comm.h"
#include "task.h"
#include "inner_task_err.h"

STATIC TASK_CB SECTION_KERNEL_INIT_DATA g_tasks[OSZ_CFG_TASK_LIMIT] = { 0 };

STATIC UINT32 inner_check_params(TASK_PARAMS *params)
{
    ASSERT(params);
    if (params->name == NULL) {
        return TASK_CREATE_CHECK_NAME_ERR;
    }
    if (!IS_ALIGN(params->stack_top, 16)) {
        return TASK_CREATE_CHECK_STACK_AGLIGN_ERR;
    }
    if (params->stack_size <= 0) {
        return TASK_CREATE_CHECK_STACK_SIZE_ERR;
    }
    return OS_OK;
}

UINT32 os_create_task(TASK_PARAMS *params)
{
    UINT32 check_ret = OS_OK;
    check_ret = inner_check_params(params);
    if (check_ret != OS_OK) {
        return OS_NOK;
    }
    UINT32 task_id;
    for (task_id = 0; task_id < OSZ_CFG_TASK_LIMIT; ++task_id) {
        if (g_tasks[task_id].used == TASK_USED) {
            continue;
        }
        g_tasks[task_id].used = TASK_USED;
        g_tasks[task_id].tsk_name = params->name;
        g_tasks[task_id].tsk_status.task_stack_top = params->stack_top;
        g_tasks[task_id].tsk_status.task_stack_length = params->stack_size;
        g_tasks[task_id].tsk_status.task_flags = TSK_FLAG_READY;
        g_tasks[task_id].tsk_context.ra = (UINT32)params->thread;
        g_tasks[task_id].tsk_context.a0 = (UINT32)params->data;
        g_tasks[task_id].tsk_context.sp = params->stack_top;
        break;
    }
    if (task_id < 0 || task_id >= OSZ_CFG_TASK_LIMIT) {
        return OS_NOK;
    }
    return task_id;
}

VOID os_delete_task(UINT32 task_id)
{
    g_tasks[task_id].used = TASK_NO_USED;
    g_tasks[task_id].tsk_name = "";
    memset(&(g_tasks[task_id].tsk_context), 0, sizeof(GP_REG));
    memset(&(g_tasks[task_id].tsk_context), 0, sizeof(TASK_STATUS));
}

VOID os_change_task_status(UINT32 task_id, TASK_FLAGS flags)
{
    TASK_CB *task = os_get_taskcb_by_tid(task_id);
    task->tsk_status.task_flags = flags;
}

UINT32 os_get_current_tid()
{
    for (int i = 0; i < OSZ_CFG_TASK_LIMIT; ++i) {
        if (g_tasks[i].tsk_status.task_flags == TSK_FLAG_RUNNING) {
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