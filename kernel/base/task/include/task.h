#ifndef __TASK_H__
#define __TASK_H__
#include "comm.h"
#include "arch_adapter.h"
#include "dlink.h"

/*
* 下一步计划：
* 1. 优化 TASK_CB，将 context 信息与任务栈合并，保持内存紧凑，并实现对应的操作函数
* 2. 增加全局 pending list，timeout list 以及 free_taskck list用于管理任务，并实现对应的操作函数
* 3. TASK_CB中增加 prority，并实现对应的操作函数
* 4. 删除当前 TASC_CB 中的 used、tsk_context 字段，并增加 topofstack、stackpoint、stacksize、taskentry字段
* 5. 注意stackpoint需要在 TASK_CB 的第一个字段
*/

typedef void (*TASK_THREAD_TYPE)(VOID *data);

#define TASK_CB_SIZE        (sizeof(TASK_CB))

#define TASK_SORTLINK_LIST(sortlink)        ((sortlink).list)
#define TASK_PSORTLINK_LIST(psortlink)      ((psortlink)->list)

#define TASK_INT_LOCK() do { \
    UINT32 intSave = 0; \
    ARCH_INT_LOCK(intSave); \
    (VOID)intSave; \
} while(0)

#define TASK_INT_UNLOCK() do { \
    UINT32 intSave = 0; \
    ARCH_INT_UNLOCK(intSave); \
    (VOID) intSave; \
} while(0)

#define TASK_CONTEXT_INIT_BASE_VALUE        0x00000000
#define TASK_CONTEXT_INIT_STEP              0x01010101
#define TASK_STACK_INIT_MAGIC               0x5A5AA5A5
#define TASK_MSTATUS_M                      (MSTATUS_MPIE | MSTATUS_MIE)
#define TASK_PRIORITY_DEFAULT               OSZ_CFG_TASK_PRI_LOWEST

#define TASK_NAME(task_id)                  (g_tasks[(task_id)].tsk_name)
#define TASK_THREAD(task_id)                (g_tasks[(task_id)].tsk_entry)
#define TASK_PRIORITY(task_id)              (g_tasks[(task_id)].tsk_priority)
#define TASK_STATE(task_id)                 (g_tasks[(task_id)].tsk_state)
#define TASK_DATA(task_id)                  (g_tasks[(task_id)].data)
#define TASK_STACK_ATTR(task_id)            (g_tasks[(task_id)].tsk_stack_attr)
#define TASK_LIST(task_id, list_name)       (g_tasks[(task_id)].tsk_list_##list_name)
#define TASK_STATE_CLEAN(task_id, st)       (g_tasks[(task_id)].tsk_state &= (~(st)))
#define TASK_STATE_SET(task_id, st)         (g_tasks[(task_id)].tsk_state |= (st))
#define TASK_GET_TASKID_BY_TASKCB(ptask_cb) (((char *)(ptask_cb) - (char *)&(g_tasks[0]))/sizeof(TASK_CB))

typedef enum {
    TSK_FLAG_FREE       = 1,
    TSK_FLAG_RUNNING    = 2,
    TSK_FLAG_READY      = 4,
    TSK_FLAG_PENDING    = 8,
    TSK_FLAG_BLOCKING   = 16,
} TASK_FLAGS;

typedef struct _GP_REG_{
    UINT32  s0;
    UINT32  s1;
    UINT32  s2;
    UINT32  s3;
    UINT32  s4;
    UINT32  s5;
    UINT32  s6;
    UINT32  s7;
    UINT32  s8;
    UINT32  s9;
    UINT32  s10;
    UINT32  s11;

    UINT32  mepc;
    UINT32  mstatus;
    UINT32  reserv_1;
    UINT32  reserv_2;

    UINT32  t6;
    UINT32  t5;
    UINT32  t4;
    UINT32  t3;
    UINT32  a7;
    UINT32  a6;
    UINT32  a5;
    UINT32  a4;
    UINT32  a3;
    UINT32  a2;
    UINT32  a1;
    UINT32  a0;
    UINT32  t2;
    UINT32  t1;
    UINT32  t0;
    UINT32  ra;
} TASK_CONTEXT;

typedef struct {
    DLINK_NODE  list;
    UINT32      timeout;
} TASK_SORT_LINK;

typedef enum {
    STACK_MEM_STATIC = 0,
    STACK_MEM_DYNAMIC = 1
} TASK_STACK_ATTR;

typedef struct {
    UINTPTR             tsk_context_pointer;        // save context info, must need to be first
    CHAR                *tsk_name;
    UINTPTR             tsk_stack_top;
    TASK_THREAD_TYPE    tsk_entry;
    UINT32              tsk_stack_size : 16;
    UINT32              tsk_priority : 16;
    VOID                *data;                      // usually, this filed use to tsk_entry params. if task is used param, this field life must be keeping to task end, suggest defined with malloc.
    UINT32              tsk_state : 8;              // type TASK_FLAGS
    UINT32              tsk_stack_attr : 1;         // task stack attrubute, like: static mem or dynamic mem.
    UINT32              tsk_reserve : 23;           // reserve field, used to keep TASK_CB align to 32bit.
    union {
        DLINK_NODE      tsk_list_free;              // free task list, because task only one state at same time.
        DLINK_NODE      tsk_list_pending;           // pending task list
        DLINK_NODE      tsk_list_ready;             // ready task list
    };
    TASK_SORT_LINK      tsk_list_blocking;          // specially, block state need to check timeout, so use TASK_SORT_LINK.
    
} TASK_CB;                                          // task base cost 44 bytes

typedef struct {
    CHAR                *name;
    UINTPTR             stack_base;
    UINT32              stack_size;
    TASK_STACK_ATTR     stack_attr;
    TASK_THREAD_TYPE    thread;
    VOID                *data;                      // if task has any data for params, this field must be keeping life cycle.
} TASK_PARAMS;

/*
* 后续优化task模块会使用下边的数据结构，主要将task模块与其他模块解耦，使用钩子的方式
*/
// typedef enum {
//     TASK_HOOK_SCHEDULE = 0,
//     TASK_HOOK_MEM_ALLOC,
//     TASK_HOOK_MEM_FREE,
//     TASK_HOOK_END,
// } TASK_HOOK;
// typedef VOID (*task_hook_callback_type)(VOID *);

// typedef struct {
//     task_hook_callback_type func;
//     VOID                    *func_param;
// } TASK_HOOK_INFO;

// extern TASK_HOOK_INFO g_ext_task_hook_info[TASK_HOOK_END];

UINT32  os_create_task(UINT16 *task_id, TASK_PARAMS *params);
VOID    os_delete_task(UINT32 task_id);
UINT32  os_get_current_tid(VOID);
TASK_CB *os_get_taskcb_by_tid(UINT32 tid);
VOID    os_change_task_status(UINT32 task_id, TASK_FLAGS flags);
VOID    os_udelay(UINT32 usec);

extern TASK_CB *gp_new_task;
extern TASK_CB *gp_current_task;
extern TASK_CB g_tasks[OSZ_CFG_TASK_LIMIT];

#endif