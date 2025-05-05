#include "preemption.h"
#include "schedule.h"
#include "pri_queue.h"
#include "task.h"

BOOL g_need_preemp = FALSE;
BOOL g_int_active = FALSE;

/*
* WARNING: DONT MODIFY THIS VARIANT.
*   this variant used to keep a0 when use os_preemp_reschedule
*   becuase a0 will submit to inner_task_handler.
*/
STATIC UINT32 g_keep_a0 = 0xffffffff;

VOID SECTION_KERNEL_INIT_TEXT os_preemp_sched_init(VOID)
{
    UINT16 task_id = -1;
    UINT32 ret = task_create_idle_task(&task_id);
    if (ret != OS_OK) {
        return;
    }
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_READY);
    TASK_STATUS_SET(task_id, TSK_FLAG_RUNNING);
    pri_queue_dequeue(TASK_PRIORITY(task_id), TASK_LIST(task_id, ready));
    gp_new_task = os_get_taskcb_by_tid(task_id);
}
MODULE_INIT(os_preemp_sched_init, l2)

UINT32 os_preemp_keep_a0(VOID)
{
    return g_keep_a0;
}

BOOL is_need_preemp(VOID)
{
    UINT32 intSave = 0;
    SCHEDULE_LOCK(&intSave);
    if (!g_need_preemp) {
        SCHEDULE_UNLOCK(intSave);
        return g_need_preemp;
    }
    UINT16 cur_task_id = os_get_current_tid();
    DLINK_NODE *dn = pri_queue_top(pri_queue_get_bitmap_low_bit());
    if (dn == NULL) {
        SCHEDULE_UNLOCK(intSave);
        g_need_preemp = FALSE;
        return g_need_preemp;
    }
    TASK_CB *tmp = STRUCT_ENTRY(TASK_CB, tsk_list_ready, dn);
    UINT16 tmp_id = os_get_task_id_by_task_cb(tmp);
    if (tmp_id == cur_task_id) {
        g_need_preemp = FALSE;
    } else {
        gp_new_task = tmp;
    }
    SCHEDULE_UNLOCK(intSave);
    return g_need_preemp;
}

VOID os_preemp_set_int_active()
{
    g_int_active = TRUE;
}

VOID os_preemp_clr_int_active()
{
    g_int_active = FALSE;
}

VOID os_preemp_schedule_with_pm(VOID)
{
    UINT32 intSave = 0;
    SCHEDULE_LOCK(&intSave);
    UINT16 new_task_id = os_get_task_id_by_task_cb(gp_new_task);
    UINT16 cur_task_id = os_get_current_tid();

    TASK_STATUS_CLEAN(cur_task_id, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(cur_task_id, TSK_FLAG_READY);
    pri_queue_enqueue(TASK_PRIORITY(cur_task_id), TASK_LIST(cur_task_id, ready), EQ_MODE_TAIL);

    TASK_STATUS_CLEAN(new_task_id, TSK_FLAG_READY);
    TASK_STATUS_SET(new_task_id, TSK_FLAG_RUNNING);
    pri_queue_dequeue(TASK_PRIORITY(new_task_id), TASK_LIST(new_task_id, ready));
    SCHEDULE_UNLOCK(intSave);
}

VOID os_preemp_reschedule(VOID)
{
    UINT32 intSave = 0;
    SCHEDULE_LOCK(&intSave);
    UINT16 cur_task_id = os_get_current_tid();
    switch (TASK_STATE(cur_task_id)) {
        case TSK_FLAG_FREE:
            break;
        case TSK_FLAG_PENDING:
            break;
        case TSK_FLAG_READY:
            break;
        case TSK_FLAG_BLOCKING:
        {
            DLINK_NODE *dn = pri_queue_top(pri_queue_get_bitmap_low_bit());
            gp_new_task = STRUCT_ENTRY(TASK_CB, tsk_list_ready, dn);
            UINT16 new_task_id = os_get_task_id_by_task_cb(gp_new_task);
            TASK_STATUS_CLEAN(new_task_id, TSK_FLAG_READY);
            TASK_STATUS_SET(new_task_id, TSK_FLAG_RUNNING);
            pri_queue_dequeue(TASK_PRIORITY(new_task_id), TASK_LIST(new_task_id, ready));
            os_preemp_keep_a0();
            schedule();
            break;
        }
        case TSK_FLAG_RUNNING:
            break;
        default:
            break;
    }
    SCHEDULE_UNLOCK(intSave);
}