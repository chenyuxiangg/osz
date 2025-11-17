#include "arch_adapter.h"
#include "mutex.h"
#include "task.h"
#include "mem.h"

#define IS_PENDLIST_EMPTY(mutex)      ((mutex)->pend_list.next == &((mutex)->pend_list))

osz_mutex_t g_static_mutex_grp[OSZ_MUTEX_STATIC_MAX_CNT] = { 0 };
STATIC uint32_t module_mutex_init(void_t)
{
    for (int i = 0; i < OSZ_MUTEX_STATIC_MAX_CNT; ++i) {
        osz_obj_t *obj = (osz_obj_t *)&g_static_mutex_grp[i];
        uint32_t ret = object_init(obj, OSZ_MOD_MUTEX);
        if (ret != OS_OK) {
            return ret;
        }
        dlink_insert_tail(&(obj->module->free_obj_list), &(obj->obj_list));
    }
    return OS_OK;
}
MODULE_INIT(module_mutex_init, l1)

STATIC uint32_t inner_mutex_obj_init(osz_mutex_t *obj, uint8_t *name, uint8_t name_size, osz_ipc_obj_create_type_t create_type)
{
    obj->attr.ipc_create_type = create_type;
    obj->attr.ipc_obj_used = IPC_USED;
    obj->attr.ipc_type = IPC_MUTEX;
    obj->attr.ipc_mutex_origin_pri = 0xff;
    obj->attr.ipc_mutex_reentrant_cnt = 0;
    dlink_init(&(obj->pend_list));
    // 此处导致拷贝越界，因此需要判断入参中name_size的大小
    name_size = (name_size <= OSZ_CFG_OBJ_NAME_MAX_LENS) ? name_size : OSZ_CFG_OBJ_NAME_MAX_LENS;
    if (name != NULL && name_size != 0) {
        memcpy(obj->supper.name, name, name_size);
    }
    if (create_type == IPC_STATIC_CREATE) {
        dlink_insert_tail(&(obj->supper.module->used_obj_list), &(obj->supper.obj_list));
    }
    return OS_OK;
}

STATIC uint32_t inner_mutex_deinit(osz_mutex_t *obj)
{
    obj->attr.ipc_create_type = IPC_NO_CREATE;
    obj->attr.ipc_obj_used = IPC_NOT_USED;
    obj->attr.ipc_type = IPC_NONE_OBJECT;
    obj->supper.owner = 0xff;
    obj->attr.ipc_mutex_origin_pri = 0xff;
    obj->attr.ipc_mutex_reentrant_cnt = 0;
    memset((void_t *)obj->supper.name, 0, OSZ_CFG_OBJ_NAME_MAX_LENS);
    return OS_OK;
}

uint32_t osz_mutex_init(uint8_t *name, uint8_t name_size, uint32_t val, osz_mutex_t **outter_obj)
{
    uint32_t ret = OS_OK;
    ret = get_free_obj(OSZ_MOD_MUTEX, ((osz_obj_t **)outter_obj));
    if (ret != OS_OK) {
        goto OUT;
    }
    inner_mutex_obj_init(*outter_obj, name, name_size, IPC_STATIC_CREATE);
    (*outter_obj)->field.mutex = val;
OUT:
    return ret;
}

STATIC uint32_t inner_mutex_check(osz_mutex_t *mutex)
{
    if (mutex == NULL) {
        return MUTEX_CHECK_NULL_OBJ_ERR;
    }
    if (mutex->supper.module != &g_modules[OSZ_MOD_MUTEX]) {
        return MUTEX_CHECK_OBJ_NOT_MUTEX_ERR;
    }
    if (mutex->attr.ipc_obj_used == IPC_NOT_USED) {
        return MUTEX_CHECK_OBJ_NOT_USED_ERR;
    }
    if (mutex->attr.ipc_type != IPC_MUTEX) {
        return MUTEX_CHECK_NOT_MUTEX_TYPE_ERR;
    }
    return OS_OK;
}

uint32_t osz_mutex_create(uint8_t *name, uint8_t name_size, osz_mutex_t **outter_obj)
{
    if (*outter_obj != NULL) {
        return MUTEX_CREATE_OUTTER_NOT_NULL_ERR;
    }
    *outter_obj = (osz_mutex_t *)osz_malloc(sizeof(osz_mutex_t));
    if (*outter_obj == NULL) {
        return MUTEX_CREATE_OUTTER_MALLOC_FAIL_ERR;
    }
    inner_mutex_obj_init(*outter_obj, name, name_size, IPC_DYNAMIC_CREATE);
    return OS_OK;
}

uint32_t osz_mutex_pend(osz_mutex_t *mutex, uint32_t timeout)
{
    uint32_t status = inner_mutex_check(mutex);
    if (status != OS_OK) {
        return status;
    }
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    uint16_t tid = osz_get_current_tid();
    uint32_t cur_task_pri = osz_get_task_priotity_by_tid(tid);
    if (mutex->field.mutex > 0) {
        mutex->field.mutex = 0;
        mutex->attr.ipc_mutex_reentrant_cnt++;
        mutex->supper.owner = tid;
        mutex->attr.ipc_mutex_origin_pri = cur_task_pri;
        ARCH_INT_UNLOCK(intsave);
        return OS_OK;
    }

    if (mutex->field.mutex == 0) {
        if (mutex->attr.ipc_mutex_origin_pri < cur_task_pri) {
            osz_set_task_priotity_by_tid(mutex->supper.owner, cur_task_pri);
            ARCH_INT_UNLOCK(intsave);
            return OS_OK;
        }
    }

    // mutex support reentrant.
    if (mutex->supper.owner == tid) {
        mutex->attr.ipc_mutex_reentrant_cnt++;
        ARCH_INT_UNLOCK(intsave);
        return OS_OK;
    }
    if (timeout == 0) {
        ARCH_INT_UNLOCK(intsave);
        return MUTEX_PEND_NO_WAIT_ERR;
    }
    TASK_STATUS_CLEAN(tid, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(tid, TSK_FLAG_PENDING);
    dlink_insert_tail(&(mutex->pend_list), TASK_LIST(tid, pending));
    ARCH_INT_UNLOCK(intsave);
    status = osz_task_wait(timeout);
    return status;
}

uint32_t osz_mutex_post(osz_mutex_t *mutex)
{
    uint32_t status = inner_mutex_check(mutex);
    if (status != OS_OK) {
        return status;
    }
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    uint16_t tid = osz_get_current_tid();
    uint16_t cur_task_pri = osz_get_task_priotity_by_tid(tid);
    if (mutex->field.mutex >= 1) {
        ARCH_INT_UNLOCK(intsave);
        return MUTEX_POST_NUM_OVER_ERR;
    }
    if (mutex->attr.ipc_mutex_reentrant_cnt > 0) {
        mutex->attr.ipc_mutex_reentrant_cnt--;
    }
    if (mutex->attr.ipc_mutex_reentrant_cnt == 0) {
        if (mutex->attr.ipc_mutex_origin_pri < cur_task_pri) {
            osz_set_task_priotity_by_tid(tid, mutex->attr.ipc_mutex_origin_pri);
        }
    }
    if (!IS_PENDLIST_EMPTY(mutex)) {
        osz_list_t *pos = mutex->pend_list.next;
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        if (task == NULL) {
            ARCH_INT_UNLOCK(intsave);
            return MUTEX_POST_WAIT_TASK_ERR;
        }
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
        ARCH_INT_UNLOCK(intsave);
        return OS_OK;
    }
    mutex->field.mutex = 1;
    ARCH_INT_UNLOCK(intsave);
    return OS_OK;
}

uint32_t osz_mutex_delete(osz_mutex_t *mutex)
{
    if (mutex == NULL) {
        return MUTEX_DELETE_NULL_OBJ_ERR;
    }
    osz_list_t *pos = NULL;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    // 考虑后续增加标识说明唤醒原因为删除mutex
    DLINK_FOREACH(pos, &(mutex->pend_list)) {
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
    }
    ARCH_INT_UNLOCK(intsave);
    osz_free(mutex);
    mutex = NULL;
    return OS_OK;
}

uint32_t osz_mutex_detach(osz_mutex_t *mutex)
{
    if (mutex == NULL) {
        return MUTEX_DELETE_NULL_OBJ_ERR;
    }
    osz_list_t *pos = NULL;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    DLINK_FOREACH(pos, &(mutex->pend_list)) {
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
    }
    inner_mutex_deinit(mutex);
    dlink_del_node(&(mutex->supper.obj_list));
    dlink_insert_tail(&(mutex->supper.module->free_obj_list), &(mutex->supper.obj_list));
    ARCH_INT_UNLOCK(intsave);
    return OS_OK;
}
