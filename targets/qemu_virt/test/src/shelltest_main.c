#include "comm.h"
#include "task.h"
#include "hook.h"
#include "csr.h"
#include "platform.h"
#include "uart.h"
#include "shell.h"
#include "schedule.h"

/* ============================================================================
 * shelltest 镜像的板级入口
 *
 * 这是「产品形态 + 测试能力」的镜像：跑起来就是正常的 OSZ（有 shell、有
 * tick、有 idle），不自动跑套件；开发者在串口里敲
 *
 *     OSZ$ test                列出全部用例
 *     OSZ$ test task.current_identity
 *
 * 手动单跑一条，用于现场定位。
 *
 * 与产品 target_qemu 的关系：产品镜像 src/main.c 一行不动，本文件是它去掉
 * demo 线程/命令后的等价骨架（g_hook_table 与中断处理必须与产品保持一致，
 * 否则 printf 走不到同一个 hook 路径）。
 * ========================================================================= */

osz_hook_entry_t g_hook_table[HOOK_ID_MAX] = {
    {HOOK_ID_PRINT, (uintptr_t)printf},
};

STATIC void shelltest_uart_recv_handler(void_t *args)
{
    shell_write_fifo(*(int8_t *)args);
}

void_t app_main(void_t *data)
{
    (void)data;
    while (1) {
        osz_msleep(5000);
    }
}

int32_t main(int32_t argc, int8_t *argv[])
{
    (void)argc;
    (void)argv;

    drv_uart_set_int_handle((interrupt_callback)shelltest_uart_recv_handler);
    first_schedule();

    while (1) {
    }
    return 0;
}
