#include "comm.h"
#include "csr.h"
#include "task.h"
#include "schedule.h"
#include "uart.h"
#include "platform.h"
#include "mem.h"

VOID test_thread1_handle(VOID)
{
    while(1) {
        DEBUG_INFO();
        os_msleep(6000);
    }
}

VOID test_thread2_handle(VOID)
{
    while(1) {
        DEBUG_INFO();
        os_msleep(3000);
    }
}

VOID test_create_task(char *task_name, TASK_THREAD_TYPE fn)
{
    UCHAR *task_stack = osz_malloc(0x800);
    TASK_PARAMS params = {
        .name = task_name,
        .stack_attr = STACK_MEM_DYNAMIC,
        .stack_base = (UINTPTR)task_stack,
        .stack_size = 0x800,
        .priority = 0x9,
        .thread = fn,
        .data = NULL
    };
    UINT16 task_id;
    UINT32 ret = os_create_task(&task_id, &params);
    if (ret != OS_OK) {
        printf("ret: %#x\n", ret);
        return;
    }
    os_task_resume(task_id);
}

INT32 main(INT32 argc, CHAR *argv[])
{
    printf("==== Enter Main ====\n");
    test_create_task("test1", (TASK_THREAD_TYPE)test_thread1_handle);
    test_create_task("test2", (TASK_THREAD_TYPE)test_thread2_handle);
    first_schedule();
    while(1);
    return 0;
}