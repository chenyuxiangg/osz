#include "sem.h"
#include "mem.h"
#include "task.h"

#define SEM_MAX_COUNT               (0xffffffff)
#define IS_PENDLIST_EMPTY(sem)      ((sem)->pend_list.next == &((sem)->pend_list))

osz_sem_t g_static_sem_grp[OSZ_SEM_STATIC_MAX_CNT] = { 0 };

STATIC uint32_t module_sem_init(void_t)
{
    for (int i = 0; i < OSZ_SEM_STATIC_MAX_CNT; ++i) {
        osz_obj_t *obj = (osz_obj_t *)&g_static_sem_grp[i];
        uint32_t ret = object_init(obj, OSZ_MOD_SEM, NULL, 0);
        if (ret != OS_OK) {
            return ret;
        }
        dlink_insert_tail(&(obj->module->free_obj_list), &(obj->obj_list));
    }
    return OS_OK;
}
MODULE_INIT(module_sem_init, l1)

STATIC uint32_t inner_sem_obj_init(osz_sem_t *obj, uint8_t *name, uint8_t name_size, osz_ipc_obj_create_type_t create_type)
{
    obj->attr.ipc_create_type = create_type;
    obj->attr.ipc_obj_used = IPC_USED;
    obj->attr.ipc_type = IPC_SEM;
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

STATIC uint32_t inner_sem_check(osz_sem_t *sem)
{
    if (sem == NULL) {
        return SEM_PEND_NULL_OBJ_ERR;
    }
    if (sem->supper.module != &g_modules[OSZ_MOD_SEM]) {
        return SEM_PEND_OBJ_NOT_SEM_ERR;
    }
    if (sem->attr.ipc_obj_used == IPC_NOT_USED) {
        return SEM_PEND_OBJ_NOT_USED_ERR;
    }
    if (sem->attr.ipc_type != IPC_SEM) {
        return SEM_PEND_NOT_SEM_TYPE_ERR;
    }
    return OS_OK;
}

uint32_t osz_sem_init(uint8_t *name, uint8_t name_size, uint32_t val, osz_sem_t **outter_obj)
{
    uint32_t ret = OS_OK;
    ret = get_free_obj(OSZ_MOD_SEM, ((osz_obj_t **)outter_obj));
    if (ret != OS_OK) {
        goto OUT;
    }
    inner_sem_obj_init(*outter_obj, name, name_size, IPC_STATIC_CREATE);
    (*outter_obj)->field.sem = val;
OUT:
    return ret;
}

uint32_t osz_sem_pend(osz_sem_t *sem, uint32_t timeout)
{
    uint32_t status = inner_sem_check(sem);
    if (status != OS_OK) {
        return status;
    }
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    if (sem->field.sem > 0) {
        sem->field.sem--;
        ARCH_INT_UNLOCK(intsave);
        return OS_OK;
    }
    if (timeout == 0) {
        ARCH_INT_UNLOCK(intsave);
        return SEM_PEND_NO_WAIT_ERR;
    }
    uint16_t tid = osz_get_current_tid();
    TASK_STATUS_CLEAN(tid, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(tid, TSK_FLAG_PENDING);
    dlink_insert_tail(&(sem->pend_list), TASK_LIST(tid, pending));
    ARCH_INT_UNLOCK(intsave);
    status = osz_task_wait(timeout);
    return status;
}

uint32_t osz_sem_post(osz_sem_t *sem)
{
    uint32_t status = inner_sem_check(sem);
    if (status != OS_OK) {
        return status;
    }
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    if (sem->field.sem >= SEM_MAX_COUNT) {
        ARCH_INT_UNLOCK(intsave);
        return SEM_POST_NUM_OVER_ERR;
    }
    if (!IS_PENDLIST_EMPTY(sem)) {
        osz_list_t *pos = sem->pend_list.next;
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        if (task == NULL) {
            ARCH_INT_UNLOCK(intsave);
            return SEM_POST_WAIT_TASK_ERR;
        }
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
        ARCH_INT_UNLOCK(intsave);
        return OS_OK;
    }
    sem->field.sem++;
    ARCH_INT_UNLOCK(intsave);
    return OS_OK;
}

uint32_t osz_sem_create(uint8_t *name, uint8_t name_size, uint32_t val, osz_sem_t **outter_obj)
{
    if (*outter_obj != NULL) {
        return SEM_CREATE_OUTTER_NOT_NULL_ERR;
    }
    *outter_obj = (osz_sem_t *)osz_malloc(sizeof(osz_sem_t));
    if (*outter_obj == NULL) {
        return SEM_CREATE_OUTTER_MALLOC_FAIL_ERR;
    }
    inner_sem_obj_init(*outter_obj, name, name_size, IPC_DYNAMIC_CREATE);
    (*outter_obj)->field.sem = val;
    return OS_OK;
}

uint32_t osz_sem_delete(osz_sem_t *sem)
{
    if (sem == NULL) {
        return SEM_DELETE_NULL_OBJ_ERR;
    }
    osz_list_t *pos = NULL;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    // 考虑后续增加标识说明唤醒原因为删除sem
    DLINK_FOREACH(pos, &(sem->pend_list)) {
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
    }
    ARCH_INT_UNLOCK(intsave);
    osz_free(sem);
    sem = NULL;
    return OS_OK;
}

uint32_t osz_sem_detach(osz_sem_t *sem)
{
    if (sem == NULL) {
        return SEM_DELETE_NULL_OBJ_ERR;
    }
    osz_list_t *pos = NULL;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    DLINK_FOREACH(pos, &(sem->pend_list)) {
        osz_task_t *task = STRUCT_ENTRY(osz_task_t, tsk_list_pending, pos);
        uint16_t tid = osz_get_task_id_by_task_cb(task);
        osz_task_wake(tid);
    }
    sem->attr.ipc_obj_used = IPC_NOT_USED;
    dlink_del_node(&(sem->supper.obj_list));
    dlink_insert_tail(&(sem->supper.module->free_obj_list), &(sem->supper.obj_list));
    ARCH_INT_UNLOCK(intsave);
    return OS_OK;
}