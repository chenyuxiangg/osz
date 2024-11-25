#include "uart.h"
#include "hal_uart.h"

static unsigned int drv_get_real_datalen_from_param(const UART_PARAMS params)
{
    return (params.data_len + UART_DATA_LEN_BASE);
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
}

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