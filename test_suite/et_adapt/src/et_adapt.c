#include "et.h"
#include "hal_uart.h"

void ET_onInit(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    return;
}

void ET_onPrintChar(char const ch)
{
    hal_uart_putc(ch);
}

void ET_onExit(int err)
{
    (void)err;
    while(1);
}