#ifndef __TASK_NEW_H__
#define __TASK_NEW_H__
#include "comm.h"
#include "arch_adapter.h"
#include "dlink.h"
#include "sortlink.h"

typedef void_t (*task_callback_t)(void_t *data);

#define TASK_CB_SIZE        (sizeof(osz_task_t))

#define TASK_CONTEXT_INIT_BASE_VALUE        0x00000000
#define TASK_CONTEXT_INIT_STEP              0x01010101
#define TASK_STACK_INIT_MAGIC               0x5A5AA5A5
#define TASK_MSTATUS_M                      (MSTATUS_MPIE | MSTATUS_MIE)
#define TASK_PRIORITY_DEFAULT               OSZ_CFG_TASK_PRI_LOWEST
#define WAIT_FOREVER                        (0xFFFFFFFF)

#define TASK_NAME(task_id)                  (g_tasks[(task_id)].tsk_name)
#define TASK_THREAD(task_id)                (g_tasks[(task_id)].tsk_entry)
#define TASK_PRIORITY(task_id)              (g_tasks[(task_id)].tsk_priority)
#define TASK_STATE(task_id)                 (g_tasks[(task_id)].tsk_state)
#define TASK_DATA(task_id)                  (g_tasks[(task_id)].data)
#define TASK_STACK_ATTR(task_id)            (g_tasks[(task_id)].tsk_stack_attr)
#define TASK_LIST(task_id, list_name)       ((osz_list_t *)&(g_tasks[(task_id)].tsk_list_##list_name))
#define TASK_STATUS_CLEAN(task_id, st)       (g_tasks[(task_id)].tsk_state &= (~(st)))
#define TASK_STATUS_SET(task_id, st)         (g_tasks[(task_id)].tsk_state |= (st))
#define TASK_GET_TASKID_BY_TASKCB(ptask_cb) (((int8_t *)(ptask_cb) - (int8_t *)&(g_tasks[0]))/sizeof(osz_task_t))
#define TASK_EVENT_OP(task_id)              (g_tasks[(task_id)].tsk_event_op)
#define TASK_EVENT_WAIT(task_id)            (g_tasks[(task_id)].tsk_event_wait_set)
#define TASK_EVENT_OCCUR(task_id)           (g_tasks[(task_id)].tsk_event.event_occur)

#define TASK_INT_LOCK(pintSave) do { \
    ARCH_INT_LOCK(*pintSave); \
    (void_t)intSave; \
} while(0)

#define TASK_INT_UNLOCK(intSave) do { \
    ARCH_INT_UNLOCK(intSave); \
} while(0)

typedef enum __osz_task_flags_t {
    TSK_FLAG_FREE       = 1,
    TSK_FLAG_RUNNING    = 2,
    TSK_FLAG_READY      = 4,
    TSK_FLAG_PENDING    = 8,        // 不支持超时
    TSK_FLAG_BLOCKING   = 16,       // 支持超时唤醒
} osz_task_flags_t;

typedef struct __osz_task_context_t{
    uint32_t  s0;
    uint32_t  s1;
    uint32_t  s2;
    uint32_t  s3;
    uint32_t  s4;
    uint32_t  s5;
    uint32_t  s6;
    uint32_t  s7;
    uint32_t  s8;
    uint32_t  s9;
    uint32_t  s10;
    uint32_t  s11;

    uint32_t  mepc;
    uint32_t  mstatus;
    uint32_t  reserv_1;
    uint32_t  reserv_2;

    uint32_t  t6;
    uint32_t  t5;
    uint32_t  t4;
    uint32_t  t3;
    uint32_t  a7;
    uint32_t  a6;
    uint32_t  a5;
    uint32_t  a4;
    uint32_t  a3;
    uint32_t  a2;
    uint32_t  a1;
    uint32_t  a0;
    uint32_t  t2;
    uint32_t  t1;
    uint32_t  t0;
    uint32_t  ra;
} osz_task_context_t;

typedef enum __osz_task_stack_attr_t{
    STACK_MEM_STATIC = 0,
    STACK_MEM_DYNAMIC = 1
} osz_task_stack_attr_t;

typedef struct __osz_task_t {
    uintptr_t             tsk_context_pointer;        // save context info, must need to be first
    int8_t                *tsk_name;
    uintptr_t             tsk_stack_top;
    task_callback_t       tsk_entry;
    uint32_t              tsk_stack_size : 16;
    uint32_t              tsk_priority : 16;
    void_t                *data;                      // usually, this filed use to tsk_entry params. if task is used param, this field life must be keeping to task end, suggest defined with malloc.
    uint32_t              tsk_state : 8;              // type osz_task_flags_t
    uint32_t              tsk_stack_attr : 1;         // task stack attrubute, like: static mem or dynamic mem.
    uint32_t              tsk_reserve : 23;           // reserve field, used to keep osz_task_t align to 32bit.
    union {
        osz_list_t        tsk_list_free;              // free task list, because task only one state at same time.
        osz_list_t        tsk_list_pending;           // pending task list
        osz_list_t        tsk_list_ready;             // ready task list
    };
    osz_sortlink_t        tsk_list_blocking;          // specially, block state need to check timeout, so use TASK_SORT_LINK.
    uint32_t              tsk_event_wait_set;
    uint32_t              tsk_event_op;
} osz_task_t;

typedef struct __osz_task_params_t {
    int8_t                  *name;
    uintptr_t               stack_base;
    uint32_t                stack_size;
    osz_task_stack_attr_t   stack_attr;
    task_callback_t         thread;
    uint32_t                priority;
    void_t                  *data;                      // if task has any data for params, this field must be keeping life cycle.
} osz_task_params_t;

/*
* 后续优化task模块会使用下边的数据结构，主要将task模块与其他模块解耦，使用钩子的方式
*/
// typedef enum {
//     TASK_HOOK_SCHEDULE = 0,
//     TASK_HOOK_MEM_ALLOC,
//     TASK_HOOK_MEM_FREE,
//     TASK_HOOK_END,
// } TASK_HOOK;
// typedef void_t (*task_hook_callback_type)(void_t *);

// typedef struct {
//     task_hook_callback_type func;
//     void_t                    *func_param;
// } TASK_HOOK_INFO;

// extern TASK_HOOK_INFO g_ext_task_hook_info[TASK_HOOK_END];

uint32_t    osz_create_task(uint16_t *task_id, osz_task_params_t *params);
void_t      osz_delete_task(uint32_t task_id);
uint32_t    osz_create_idle_task(uint16_t *task_id);
uint32_t    osz_get_current_tid(void_t);
osz_task_t  *osz_get_taskcb_by_tid(uint32_t tid);
uint32_t    osz_get_task_priotity_by_tid(uint32_t tid);
uint32_t    osz_set_task_priotity_by_tid(uint32_t tid, uint32_t new_pri);
uint16_t    osz_get_task_id_by_task_cb(osz_task_t *task);
void_t      osz_udelay(uint64_t usec);
void_t      osz_msleep(uint64_t msec);
void_t      osz_update_task(void_t);
void_t      osz_task_suspend(uint16_t task_id);
void_t      osz_task_resume(uint16_t task_id);
void_t      osz_task_yeild();
uint32_t    osz_task_wait(uint64_t timeout);
uint32_t    osz_task_wake(uint16_t task_id);

extern osz_task_t *gp_new_task;
extern osz_task_t *gp_current_task;
extern osz_task_t g_tasks[OSZ_CFG_TASK_LIMIT];
extern osz_sortlink_t g_task_sortlink;

#endif