#include "comm.h"
#include "csr.h"
#include "task.h"
#include "hook.h"
#include "schedule.h"
#include "uart.h"
#include "platform.h"
#include "mem.h"
#include "event.h"
#include "shell.h"

osz_events_t *g_ecb = NULL;

osz_hook_entry_t g_hook_table[HOOK_ID_MAX] = {
    {HOOK_ID_PRINT, (uintptr_t)printf},
};

void_t test_thread1_handle(void_t)
{
    uint32_t recv_event = 0;
    DEBUG_INFO();
    while(1) {
        uint32_t res = osz_events_read(g_ecb, 1, EVENT_FLAG_OR | EVENT_FLAG_CLEAN, WAIT_FOREVER, &recv_event);
        printf("task1 read event ok. res=%#x\n", res);
        osz_events_write(g_ecb, 2);
        osz_msleep(6000);
    }
}

void_t test_thread2_handle(void_t)
{
    uint32_t recv_event = 0;
    DEBUG_INFO();
    while(1) {
        uint32_t res = osz_events_write(g_ecb, 1);
        osz_events_read(g_ecb, 2, EVENT_FLAG_OR | EVENT_FLAG_CLEAN, WAIT_FOREVER, &recv_event);
        printf("task2 read event ok. res=%#x\n", res);
        osz_msleep(3000);
    }
}

void_t test_create_task(int8_t *task_name, task_callback_t fn, uint32_t pri)
{
    uint8_t *task_stack = osz_malloc(0x800);
    osz_task_params_t params = {
        .name = task_name,
        .stack_attr = STACK_MEM_DYNAMIC,
        .stack_base = (UINTPTR)task_stack,
        .stack_size = 0x800,
        .priority = pri,
        .thread = fn,
        .data = NULL
    };
    uint16_t task_id;
    uint32_t ret = osz_create_task(&task_id, &params);
    if (ret != OS_OK) {
        printf("ret: %#x\n", ret);
        return;
    }
    osz_task_resume(task_id);
}

void_t test_uart_recv_handler(void_t *args)
{
    if (*(int8_t *)args == 0xd) {
        printf("\n");
    } else {
        printf("%c", *(int8_t *)args);
    }
    shell_write_fifo(*(int8_t *)args);
}

void_t test_cmd_func(uint32_t argc, int8_t *argv[])
{
    if (argc < 0 || argc > 3) {
        printf("err args.\n");
        return;
    }
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
}

void_t app_main(void_t *) {
    while(1) {
        printf("%s, %d\n", __FUNCTION__, __LINE__);
        osz_msleep(5000);   
    }
}

int32_t main(int32_t argc, int8_t *argv[])
{
    printf("==== Enter Main ====\n");
    uint32_t res = osz_events_init(NULL, 0, &g_ecb);
    if (res != OS_OK) {
        DEBUG_INFO();
        return OS_NOK;
    }
    drv_uart_set_int_handle((interrupt_callback)test_uart_recv_handler);
    CMD_PARAMS params1 = {
        .argc = 3,
        .cmd_func = (CMD_CALLBACK_FUNC)test_cmd_func,
        .cmd_name = "test"
    };
    CMD_PARAMS params2 = {
        .argc = 2,
        .cmd_func = (CMD_CALLBACK_FUNC)test_cmd_func,
        .cmd_name = "cyx"
    };
    CMD_PARAMS params3 = {
        .argc = 2,
        .cmd_func = (CMD_CALLBACK_FUNC)test_cmd_func,
        .cmd_name = "tea"
    };
    shell_register_cmd(&params1);
    shell_register_cmd(&params2);
    shell_register_cmd(&params3);
    // test_create_task("test1", (task_callback_t)test_thread1_handle, 0x9);
    // test_create_task("test2", (task_callback_t)test_thread2_handle, 0xa);
    first_schedule();
    while(1);
    return 0;
}