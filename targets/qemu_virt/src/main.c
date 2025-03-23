#include "comm.h"
#include "csr.h"
#include "task.h"
#include "schedule.h"
#include "uart.h"
#include "platform.h"
#include "mem.h"

extern UINT32 g_test_sched;

VOID test_thread1_handle(VOID)
{
    printf("current thread: %d\n", os_get_current_tid());
    while(1) {
        g_test_sched = 0;
        unsigned int count = 1000000000;
        while(count--);
        printf("thread1...\n");
    }
}

VOID test_thread2_handle(VOID)
{
    printf("current thread: %d\n", os_get_current_tid());
    while(1) {
        g_test_sched = 0;
        unsigned int count = 2000000000;
        while(count--);
        printf("thread2...\n");
    }
}

VOID test_create_task(char *task_name, TASK_THREAD_TYPE fn)
{
    UCHAR *task_stack = osz_malloc(0x800);
    printf("name: %s, task_stack: %p\n", task_name, task_stack);
    TASK_PARAMS params = {
        .name = task_name,
        .stack_attr = STACK_MEM_DYNAMIC,
        .stack_base = (UINTPTR)task_stack,
        .stack_size = 0x800,
        .thread = fn,
        .data = NULL
    };
    UINT16 task_id;
    UINT32 ret = os_create_task(&task_id, &params);
    if (ret != OS_OK) {
        printf("ret: %#x\n", ret);
        return;
    } else {
        printf("name: %s, id: %d\n", task_name, task_id);
    }
    TASK_STATE(task_id) &= (~TSK_FLAG_PENDING);
    TASK_STATE(task_id) |= TSK_FLAG_READY;
}

VOID test_uart_recv_int_handle(VOID *args)
{
    UCHAR ch = *(UCHAR *)args;
    switch(ch) {
        case 'a':
            gp_current_task = os_get_taskcb_by_tid(os_get_current_tid());
            gp_current_task->tsk_state = TSK_FLAG_PENDING;
            gp_new_task = os_get_taskcb_by_tid(1);
            gp_new_task->tsk_state = TSK_FLAG_RUNNING;
            g_test_sched = 1;
            break;
        case 'b':
            gp_current_task = os_get_taskcb_by_tid(os_get_current_tid());
            gp_current_task->tsk_state = TSK_FLAG_PENDING;
            gp_new_task = os_get_taskcb_by_tid(2);
            gp_new_task->tsk_state = TSK_FLAG_RUNNING;
            g_test_sched = 1;
            break;
        default:
            printf("input: %c\n", ch);
            return;
    }
}

INT32 main(INT32 argc, CHAR *argv[])
{
    printf("test\n");
    test_create_task("test1", (TASK_THREAD_TYPE)test_thread1_handle);
    test_create_task("test2", (TASK_THREAD_TYPE)test_thread2_handle);
    drv_uart_set_int_handle(test_uart_recv_int_handle);
    first_schedule();

    while(1);
    return 0;
}