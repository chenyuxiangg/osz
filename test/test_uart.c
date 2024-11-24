#include "uart.h"

void test_uart(void)
{
    drv_uart_init();
	drv_uart_write("hello,riscv!\n");
}