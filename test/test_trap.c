#include "comm.h"
#include "mem.h"
#include "task.h"
#include "schedule.h"

static void task(void *data)
{
    printf("enter trap!\n");
    *(int *)(0x0) = 0x100;
    printf("exit trap!\n");
}

void test_trap(void)
{
    os_schedule_init();
    UCHAR *stack = (UCHAR *)osz_malloc(0x800);
    TASK_PARAMS param = {
        .data = NULL,
        .name = "test_trap",
        .stack_size = 0x800,
        .stack_top = (UINTPTR)stack,
        .thread = (TASK_THREAD_TYPE)task
    };
    UINT32 tid = os_create_task(&param);
    printf("create tid: %#x success!\n", tid);
    os_schedule();
}