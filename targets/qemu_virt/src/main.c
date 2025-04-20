#include "comm.h"
#include "csr.h"
#include "task.h"
#include "schedule.h"
#include "uart.h"
#include "platform.h"
#include "mem.h"
#include "event.h"

VOID test_thread1_handle(VOID)
{
    EVENT_CB ecb = {
        .event_op = EVENT_OP_OR | EVENT_OP_CLR,
        .event_wait = 1
    };
    DEBUG_INFO();
    while(1) {
        event_read(&ecb, EVENT_WAIT_FOREVER);
        printf("task1 read event ok.\n");
        event_write(2);
        os_msleep(6000);
    }
}

VOID test_thread2_handle(VOID)
{
    EVENT_CB ecb = {
        .event_op = EVENT_OP_OR | EVENT_OP_CLR,
        .event_wait = 2
    };
    DEBUG_INFO();
    while(1) {
        event_write(1);
        event_read(&ecb, EVENT_WAIT_FOREVER);
        printf("task2 read event ok.\n");
        os_msleep(3000);
    }
}

VOID test_create_task(char *task_name, TASK_THREAD_TYPE fn, UINT32 pri)
{
    UCHAR *task_stack = osz_malloc(0x800);
    TASK_PARAMS params = {
        .name = task_name,
        .stack_attr = STACK_MEM_DYNAMIC,
        .stack_base = (UINTPTR)task_stack,
        .stack_size = 0x800,
        .priority = pri,
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

VOID test_uart_recv_handler(VOID *args)
{
    printf("%c", *(CHAR *)args);
}

INT32 main(INT32 argc, CHAR *argv[])
{
    printf("==== Enter Main ====\n");
    drv_uart_set_int_handle((interrupt_callback)test_uart_recv_handler);
    test_create_task("test1", (TASK_THREAD_TYPE)test_thread1_handle, 0x9);
    test_create_task("test2", (TASK_THREAD_TYPE)test_thread2_handle, 0xa);
    first_schedule();
    while(1);
    return 0;
}