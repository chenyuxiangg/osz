#include "comm.h"
#include "uart.h"
#include "hal_uart.h"
#include "platform.h"

interrupt_callback g_uart_int_handle = NULL;

static unsigned int drv_get_real_datalen_from_param(const UART_PARAMS params)
{
    return (params.data_len + UART_DATA_LEN_BASE);
}

static void drv_uart_int_default_handle(void *args)
{
    if ((READ_BYTE(UART0_BASE_ADDR + LSR) & LSR_RX_VALID) == 0) {
        return;
    }
	UINT8 ch = READ_BYTE(UART0_BASE_ADDR + RHR);
    if (g_uart_int_handle) {
        g_uart_int_handle((VOID *)&ch);
    }
}

void drv_uart_init(void)
{
    unsigned char irq = 0;
    hal_uart_set_irq_en(irq);
    UART_PARAMS params = {
        .parity_en = 0,
        .parity_force_en = 0,
        .stop_bit = 0,
        .data_len = UART_DL_8
    };
    drv_uart_set_params(params);
    hal_uart_set_baudrate(UART_BAUDRATE);
    hal_uart_set_irq_en(0x1);

    osz_interrupt_regist(UART_INT_NUM, drv_uart_int_default_handle);
    osz_interrupt_enable(UART_INT_NUM);
    osz_interrupt_set_pri(UART_INT_NUM, 1);
}
MODULE_INIT(drv_uart_init, l1)

void drv_uart_set_params(const UART_PARAMS params)
{
    hal_uart_set_parity_en(params.parity_en);
    hal_uart_set_parity_mode((HAL_PARITY_MODE)params.parity_mode);
    hal_uart_set_parity_force_en(params.parity_force_en);
    hal_uart_set_stop_bit(params.stop_bit);
    hal_uart_set_data_len(drv_get_real_datalen_from_param(params));
}

void drv_uart_write(const char *str)
{
    hal_uart_write(str);
}

void drv_uart_putc(const char c)
{
    hal_uart_putc(c);
}

void drv_uart_set_int_handle(interrupt_callback fn)
{
    g_uart_int_handle = fn;
}