#include "arch_adapter.h"
#include "event.h"
#include "task.h"
#include "inner_task_err.h"
#include "mem.h"

#define IS_EMPTY(pend_list)     ((pend_list)->next != (pend_list))

osz_events_t g_static_events_grp[OSZ_EVENT_STATIC_MAX_CNT] = { 0 };
STATIC uint32_t module_events_init(void_t)
{
    for (int i = 0; i < OSZ_EVENT_STATIC_MAX_CNT; ++i) {
        osz_obj_t *obj = (osz_obj_t *)&g_static_events_grp[i];
        uint32_t ret = object_init(obj, OSZ_MOD_EVENTS);
        if (ret != OS_OK) {
            return ret;
        }
        dlink_insert_tail(&(obj->module->free_obj_list), &(obj->obj_list));
    }
    return OS_OK;
}
MODULE_INIT(module_events_init, l1)

STATIC uint32_t inner_events_check(osz_events_t *events)
{
    if (events == NULL) {
        return EVENT_READ_EVENTS_NULL_ERR;
    }
    if (events->supper.module != &g_modules[OSZ_MOD_EVENTS]) {
        return EVENT_READ_EVENTS_OBJ_NOT_IPC_ERR;
    }
    if (events->attr.ipc_obj_used == IPC_NOT_USED) {
        return EVENT_READ_EVENTS_OBJ_NOT_USED_ERR;
    }
    if (events->attr.ipc_type != IPC_EVENTS) {
        return EVENT_READ_NOT_EVENTS_TYPE_ERR;
    }
    return OS_OK;
}

STATIC uint32_t inner_events_obj_init(osz_events_t *obj, uint8_t *name, uint8_t name_size, osz_ipc_obj_create_type_t create_type)
{
    obj->attr.ipc_create_type = create_type;
    obj->attr.ipc_obj_used = IPC_USED;
    obj->attr.ipc_type = IPC_EVENTS;
    dlink_init(&(obj->pend_list));
    name_size = (name_size > OSZ_CFG_OBJ_NAME_MAX_LENS) ? OSZ_CFG_OBJ_NAME_MAX_LENS : name_size;
    if (name != NULL && name_size != 0) {
        memcpy(obj->supper.name, name, name_size);
    }
    if (create_type == IPC_STATIC_CREATE) {
        dlink_insert_tail(&(obj->supper.module->used_obj_list), &(obj->supper.obj_list));
    }
    return OS_OK;
}

STATIC uint32_t inner_events_deinit(osz_events_t *obj)
{
    obj->attr.ipc_create_type = IPC_NO_CREATE;
    obj->attr.ipc_obj_used = IPC_NOT_USED;
    obj->attr.ipc_type = IPC_NONE_OBJECT;
    obj->field.events = 0;
    obj->supper.owner = 0xff;
    memset((void_t *)obj->supper.name, 0, OSZ_CFG_OBJ_NAME_MAX_LENS);
    return OS_OK;
}

uint32_t osz_events_init(uint8_t *name, uint8_t name_size, osz_events_t **outter_obj)
{
    uint32_t ret = OS_OK;
    ret = get_free_obj(OSZ_MOD_EVENTS, ((osz_obj_t **)outter_obj));
    if (ret != OS_OK) {
        goto OUT;
    }
    inner_events_obj_init(*outter_obj, name, name_size, IPC_STATIC_CREATE);
OUT:
    return ret;
}

/*
* TODO: 考虑处理如下场景
*   1. events上有任务在等待，但是events被删除的情况
*/
uint32_t osz_events_read(osz_events_t *events, uint32_t event_set, osz_events_op_t op_flag, uint32_t timeout, uint32_t *outter_events)
{
    uint32_t status = OS_NOK;
    status = inner_events_check(events);
    if (status != OS_OK) {
        return status;
    }
    if ((op_flag & (EVENT_FLAG_AND | EVENT_FLAG_OR)) == (EVENT_FLAG_AND | EVENT_FLAG_OR)) {
        return EVENT_READ_OP_CONFLICT_ERR;
    }

    status = OS_NOK;
    uint32_t intsave = 0;

    ARCH_INT_LOCK(intsave);
    if ((op_flag & EVENT_FLAG_AND) == EVENT_FLAG_AND) {
        *outter_events = events->field.events & event_set;
        if (*outter_events == event_set) {
            status = OS_OK;
        }
    } else if ((op_flag & EVENT_FLAG_OR) == EVENT_FLAG_OR) {
        *outter_events = events->field.events & event_set;
        if (*outter_events != 0) {
            status = OS_OK;
        }
    } else {
        ARCH_INT_UNLOCK(intsave);
        return EVENT_READ_OP_NOT_EXSIT_ERR;
    }
    if (status == OS_OK) {
        if ((op_flag & EVENT_FLAG_CLEAN) == EVENT_FLAG_CLEAN) {
            events->field.events &= ~(*outter_events);
        }
        ARCH_INT_UNLOCK(intsave);
        return OS_OK;
    }
    if (timeout == 0) {
        ARCH_INT_UNLOCK(intsave);
        return EVENT_READ_NO_WAIT_ERR;
    }
    uint16_t tid = osz_get_current_tid();
    TASK_EVENT_WAIT(tid) = event_set;
    TASK_EVENT_OP(tid) = op_flag;
    TASK_STATUS_CLEAN(tid, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(tid, TSK_FLAG_PENDING);
    dlink_insert_tail(&(events->pend_list), TASK_LIST(tid, pending));
    ARCH_INT_UNLOCK(intsave);
    status = osz_task_wait(timeout);

    if (status == TASK_WAIT_WAKE_UP_ERR) {
        ARCH_INT_LOCK(intsave);
        *outter_events = events->field.events & event_set;
        ARCH_INT_UNLOCK(intsave);
    }

    return status;
}

/*
* TODO: 需要考虑区分超时唤醒和写入事件唤醒
*/
uint32_t osz_events_write(osz_events_t *events, uint32_t event_set)
{
    uint32_t status = OS_NOK;
    status = inner_events_check(events);
    if (status != OS_OK) {
        return status;
    }
    events->field.events |= event_set;
    osz_list_t *pos = events->pend_list.next;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    while (pos != &(events->pend_list)) {
        osz_list_t *next = pos->next;
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        if (task == NULL) {
            ARCH_INT_UNLOCK(intsave);
            return EVENT_WRITE_WAIT_TASK_ERR;
        }
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        uint32_t recv_event = 0;
        if ((TASK_EVENT_OP(tid) & EVENT_FLAG_AND) == EVENT_FLAG_AND) {
            if ((TASK_EVENT_WAIT(tid) & events->field.events) == TASK_EVENT_WAIT(tid)) {
                recv_event = TASK_EVENT_WAIT(tid);
                osz_task_wake(tid);
            }
        } else if ((TASK_EVENT_OP(tid) & EVENT_FLAG_OR) == EVENT_FLAG_OR) {
            if ((TASK_EVENT_WAIT(tid) & events->field.events) != 0) {
                recv_event = TASK_EVENT_WAIT(tid) & events->field.events;
                osz_task_wake(tid);
            }
        }
        if ((TASK_EVENT_OP(tid) & EVENT_FLAG_CLEAN) == EVENT_FLAG_CLEAN) {
            events->field.events &= ~(recv_event);
        }
        pos = next;
    }
    ARCH_INT_UNLOCK(intsave);
    return OS_OK;
}

uint32_t osz_events_create(uint8_t *name, uint8_t name_size, osz_events_t **outter_obj)
{
    if (*outter_obj != NULL) {
        return EVENT_CREATE_OUTTER_NOT_NULL_ERR;
    }
    *outter_obj = (osz_events_t *)osz_malloc(sizeof(osz_events_t));
    if (*outter_obj == NULL) {
        return EVENT_CREATE_OUTTER_MALLOC_FAIL_ERR;
    }
    inner_events_obj_init(*outter_obj, name, name_size, IPC_DYNAMIC_CREATE);
    return OS_OK;
}

uint32_t osz_events_delete(osz_events_t *events)
{
    if (events == NULL) {
        return EVENT_DELETE_NULL_OBJ_ERR;
    }
    osz_list_t *pos = NULL;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    DLINK_FOREACH(pos, &(events->pend_list)) {
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
    }
    ARCH_INT_UNLOCK(intsave);
    osz_free(events);
    events = NULL;
    return OS_OK;
}

uint32_t osz_events_detach(osz_events_t *events)
{
    if (events == NULL) {
        return EVENT_DELETE_NULL_OBJ_ERR;
    }
    osz_list_t *pos = NULL;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    DLINK_FOREACH(pos, &(events->pend_list)) {
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
    }
    inner_events_deinit(events);
    dlink_del_node(&(events->supper.obj_list));
    dlink_insert_tail(&(events->supper.module->free_obj_list), &(events->supper.obj_list));
    ARCH_INT_UNLOCK(intsave);
    return OS_OK;
}