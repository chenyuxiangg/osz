#include "comm.h"
#include "coordination.h"

extern VOID switch_to(GP_REG *regs);
STATIC UCHAR g_init_task_stack[0x800] ATTR_ALIGN(16) = { 0 };

STATIC VOID os_switch_to(TASK_CB *new_task)
{
    ASSERT(new_task);
    switch_to(&(new_task->tsk_context));
}

STATIC TASK_CB *os_get_next_valid_task()
{
    UINT32 task_id = os_get_current_tid();
    UINT32 nid;
    TASK_CB *task = NULL;
    for (nid = (task_id + 1) % OSZ_CFG_TASK_LIMIT; nid != task_id; nid = (nid + 1) % OSZ_CFG_TASK_LIMIT) {
        task = os_get_taskcb_by_tid(nid);
        if (task->used != TASK_USED || task->tsk_status.task_flags != TSK_FLAG_READY) {
            continue;
        }
        break;
    }
    return task;
}

VOID os_coord_sched_init()
{
    TASK_PARAMS param = {
        .data = NULL,
        .name = "init",
        .stack_size = 0x800,
        .stack_top = (UINTPTR)g_init_task_stack,
        .thread = NULL
    };
    UINT32 tid = os_create_task(&param);
    os_change_task_status(tid, TSK_FLAG_RUNNING);
}

VOID os_coord_schedule()
{
    // UINT32 tid = os_get_current_tid();
    // TASK_CB *cur_task = os_get_taskcb_by_tid(tid);
    // TASK_CB *next_task = os_get_next_valid_task();
    // if (cur_task->tsk_status.task_flags == TSK_FLAG_RUNNING) {
    //     if (strncmp(cur_task->tsk_name, "init", strlen(cur_task->tsk_name)) == 0) {
    //         os_change_task_status(tid, TSK_FLAG_PENDING);
    //     } else {
    //         os_change_task_status(tid, TSK_FLAG_READY);
    //     }
    // }
    // if (next_task != NULL) {
    //     next_task->tsk_status.task_flags = TSK_FLAG_RUNNING;
    //     os_switch_to(next_task);
    // }
}