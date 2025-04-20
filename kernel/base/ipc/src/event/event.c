#include "event.h"
#include "inner_event_err.h"
#include "task.h"

#define EVENT_OP_MASK       (0xf)
#define EVENT_OP_INVALID    (0x0)
#define EVENT_WAIT_INVALID  (0x0)

STATIC BOOL inner_event_check(EVENT_CB *ecb)
{
    if ((ecb->event_op & EVENT_OP_MASK) == (EVENT_OP_OR || EVENT_OP_AND)) {
        return EVENT_OP_COMFLICT_ERR;
    }
    if ((ecb->event_op & EVENT_OP_MASK) == EVENT_OP_INVALID) {
        return EVENT_OP_INVALID_ERR;
    }
    if (ecb->event_wait == EVENT_WAIT_INVALID) {
        return EVENT_WAIT_INVALID_ERR;
    }
    return OS_OK;
}

UINT32 event_poll(EVENT_CB *ecb)
{
    ASSERT(ecb);
    UINT32 event_id = 0;
    if (inner_event_check(ecb) != OS_OK) {
        return event_id;
    }
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    if ((ecb->event_op & EVENT_OP_OR) == EVENT_OP_OR) {
        event_id = (ecb->event_wait & ecb->event_occur);
    } else {
        event_id = ((ecb->event_wait & ecb->event_occur) == ecb->event_wait) ? ecb->event_wait : 0;
    }
    if ((ecb->event_op & EVENT_OP_CLR) == EVENT_OP_CLR) {
        ecb->event_occur = ecb->event_occur & (~event_id);
    }
    TASK_INT_UNLOCK(intSave);
    return event_id;
}

UINT32 event_read(EVENT_CB *ecb, UINT32 timeout)
{
    ASSERT(ecb);
    UINT32 events = 0;
    if (timeout == 0) {
        return events;
    }
    UINT16 task_id = os_get_current_tid();
    EVENT_CB *task_event = &TASK_EVENT_CB(task_id);
    events = event_poll(task_event);
    if (events != 0) {
        return events;
    }
    os_task_wait(ecb, EVENT_WAIT_FOREVER);
    events = event_poll(task_event);
    return events;
}

VOID event_write(UINT32 events)
{
    if (events == 0) {
        return;
    }
    UINT32 intSave = 0;
    TASK_INT_LOCK(&intSave);
    DLINK_NODE *it = NULL;
    DLINK_FOREACH(it, &SORTLINK_LIST(g_task_sortlink)) {
        SORT_LINK *sl = STRUCT_ENTRY(SORT_LINK, list, it);
        TASK_CB *task_cb = STRUCT_ENTRY(TASK_CB, tsk_list_blocking, sl);
        UINT16 task_id = os_get_task_id_by_task_cb(task_cb);
        if ((((TASK_EVENT_OP(task_id) & EVENT_OP_OR) == EVENT_OP_OR) && 
            ((events & TASK_EVENT_WAIT(task_id)) != 0)) ||
            (((TASK_EVENT_OP(task_id) & EVENT_OP_AND) == EVENT_OP_AND) && 
            ((events & TASK_EVENT_WAIT(task_id)) == TASK_EVENT_WAIT(task_id)))) {
            TASK_EVENT_OCCUR(task_id) = events;
            os_task_wake(task_id);
        }
    }
    TASK_INT_UNLOCK(intSave);
}