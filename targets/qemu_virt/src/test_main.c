#include "comm.h"
#include "csr.h"
#include "task.h"
#include "schedule.h"
#include "uart.h"
#include "platform.h"
#include "mem.h"
#include "event.h"
#include "shell.h"

void_t test_uart_recv_handler(void_t *args)
{
    if (*(int8_t *)args == 0xd) {
        printf("\n");
    } else {
        printf("%c", *(int8_t *)args);
    }
    shell_write_fifo(*(int8_t *)args);
}

int32_t main(int32_t argc, int8_t *argv[])
{
    printf("==== Enter Main ====\n");
    drv_uart_set_int_handle((interrupt_callback)test_uart_recv_handler);
    first_schedule();
    while(1);
    return 0;
}