#ifndef __TASK_H__
#define __TASK_H__
#include "comm.h"

typedef void (*TASK_THREAD_TYPE)(VOID *data);

#define TASK_CB_SIZE        (sizeof(TASK_CB))
#define TASK_USED           (1)
#define TASK_NO_USED        (0)

typedef enum {
    TSK_FLAG_RUNNING = 0,
    TSK_FLAG_READY,
    TSK_FLAG_PENDING,
} TASK_FLAGS;

typedef struct _GP_REG_{
    UINT32  ra;
    UINT32  sp;
    UINT32  gp;
    UINT32  tp;
    UINT32  t0;
    UINT32  t1;
    UINT32  t2;
    UINT32  s0;
    UINT32  s1;
    UINT32  a0;
    UINT32  a1;
    UINT32  a2;
    UINT32  a3;
    UINT32  a4;
    UINT32  a5;
    UINT32  a6;
    UINT32  a7;
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
    UINT32  t3;
    UINT32  t4;
    UINT32  t5;
    UINT32  t6;
} GP_REG;

typedef struct {
    UINTPTR task_stack_top;
    UINT32 task_stack_length;
    TASK_FLAGS task_flags;
} TASK_STATUS;

typedef struct {
    CHAR                *tsk_name;
    GP_REG              tsk_context;
    TASK_STATUS         tsk_status;
    UINT32              used;
} TASK_CB;

typedef struct {
    CHAR    *name;
    UINTPTR stack_top;
    UINT32  stack_size;
    TASK_THREAD_TYPE thread;
    VOID    *data;
} TASK_PARAMS;

UINT32 os_create_task(TASK_PARAMS *params);
VOID os_delete_task(UINT32 task_id);
UINT32 os_get_current_tid(VOID);
TASK_CB *os_get_taskcb_by_tid(UINT32 tid);
VOID os_change_task_status(UINT32 task_id, TASK_FLAGS flags);
VOID os_udelay(UINT32 usec);

#endif