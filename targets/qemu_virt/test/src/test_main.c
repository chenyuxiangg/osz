#include "comm.h"
#include "task.h"
#include "hook.h"
#include "shell.h"
#include "csr.h"
#include "platform.h"
#include "mem.h"
#include "event.h"
#include "uart.h"
#include "schedule.h"
#include "osz_test.h"

#ifndef TC_BOARD_NAME
#define TC_BOARD_NAME "qemu-virt"
#endif

/* 产品镜像里 main.c 定义了这个 hook 表；测试镜像给一份等价的最小表，
 * 让 printf 走同一个 hook 路径，保证与产品行为一致。 */
osz_hook_entry_t g_hook_table[HOOK_ID_MAX] = {
    {HOOK_ID_PRINT, (uintptr_t)printf},
};

STATIC void tc_uart_recv_handler(void_t *args)
{
    shell_write_fifo(*(int8_t *)args);
}

/*
 * app_main 由 kernel 在执行 init 段之后创建为任务（静态栈，默认大小）。
 * 测试镜像把整个套件跑在这个任务里：它是 runner，也是所有用例的执行上下文。
 */
void_t app_main(void_t *data)
{
    (void)data;

    drv_uart_set_int_handle((interrupt_callback)tc_uart_recv_handler);

    int fails = tc_run_all(TC_BOARD_NAME);

    /* 哨兵：宿主 runner 见到它就判定套件结束；失败数用于人眼复核 */
    if (fails == 0) {
        printf("[SZTEST] sentinel OK\n");
    } else {
        printf("[SZTEST] sentinel FAILED\n");
    }

    while (1) {
        osz_msleep(1000);
    }
}

int32_t main(int32_t argc, int8_t *argv[])
{
    (void)argc;
    (void)argv;

    first_schedule();

    while (1) {
    }
    return 0;
}
