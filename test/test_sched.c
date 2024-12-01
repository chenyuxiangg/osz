#include "printf.h"
#include "zos_mem.h"
#include "task.h"
#include "zos_sched.h"

static void task1(void *data)
{
    printf("=== Enter task1 ===\n");
    int count = 0;
    while (1) {
        printf("[task1] do something...[%d]\n", count++);
        os_udelay(5);
        os_schedule();
    }
}

static void task2(void *data)
{
    printf("=== Enter task2 ===\n");
    int count = 0;
    while (1) {
        printf("[task2] do something...[%d]\n", count++);
        os_udelay(10);
        os_schedule();
    }
    // os_schedule();
    // while (1);
}

void test_sched(void)
{
    os_schedule_init();
    UCHAR *stack1 = (UCHAR *)zos_malloc(0x800);
    TASK_PARAMS param1 = {
        .data = NULL,
        .name = "task1",
        .stack_size = 0x800,
        .stack_top = (UINTPTR)stack1,
        .thread = (TASK_THREAD_TYPE)task1
    };
    UCHAR *stack2 = (UCHAR *)zos_malloc(0x800);
    TASK_PARAMS param2 = {
        .data = NULL,
        .name = "task2",
        .stack_size = 0x800,
        .stack_top = (UINTPTR)stack2,
        .thread = (TASK_THREAD_TYPE)task2
    };
    UINT32 tid1 = os_create_task(&param1);
    if (tid1 != OS_NOK) {
        printf("Create task1 success. tid = %d\n", tid1);
    }
    UINT32 tid2 = os_create_task(&param2);
    if (tid2 != OS_NOK) {
        printf("Create task2 success. tid = %d\n", tid2);
    }
    os_schedule();
}