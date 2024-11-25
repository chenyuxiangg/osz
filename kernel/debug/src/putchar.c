#include "uart.h"

void _putchar(char character)
{
    drv_uart_putc(character);
}