#include "platform.h"
#include "hal_uart.h"

void hal_uart_set_baudrate(unsigned int baudrate)
{
    unsigned char psd = 0;
    unsigned char dl = (UART0_CLOCK) / (16 * baudrate) / (psd + 1);

    WRITE_BYTE((UART0_BASE_ADDR + LCR), (1 << DLAB));
    WRITE_BYTE((UART0_BASE_ADDR + DLL), (dl & 0xff));
    WRITE_BYTE((UART0_BASE_ADDR + DLM), ((dl >> 8) & 0xff));
    // WRITE_BYTE((UART0_BASE_ADDR + PSD), psd);
    WRITE_BYTE((UART0_BASE_ADDR + LCR), 0x0);
}

void hal_uart_set_parity_en(unsigned char en)
{
    unsigned char lcr = READ_BYTE(UART0_BASE_ADDR + LCR);
    if (en == 0) {
        CLEAN_BIT(lcr, ENABLE_PARITY);
    } else if (en == 1) {
        SET_BIT(lcr, ENABLE_PARITY);
    } else {
        return;
    }
    WRITE_BYTE((UART0_BASE_ADDR + LCR), lcr);
}

void hal_uart_set_irq_en(unsigned char en)
{
    WRITE_BYTE((UART0_BASE_ADDR + IER), (en & 0xff));
}

void hal_uart_set_parity_mode(HAL_PARITY_MODE mode)
{
    unsigned char lcr = READ_BYTE(UART0_BASE_ADDR + LCR);
    if (mode == PARITY_ODD) {
        CLEAN_BIT(lcr, EVEN_PARITY);
    } else if (mode == PARITY_EVEN) {
        SET_BIT(lcr, EVEN_PARITY);
    } else {
        return;
    }
    WRITE_BYTE((UART0_BASE_ADDR + LCR), lcr);
}

void hal_uart_set_parity_force_en(unsigned char en)
{
    unsigned char lcr = READ_BYTE(UART0_BASE_ADDR + LCR);
    if (en == 0) {
        CLEAN_BIT(lcr, FORCE_PARITY);
    } else if (en == 1) {
        SET_BIT(lcr, FORCE_PARITY);
    } else {
        return;
    }
    WRITE_BYTE((UART0_BASE_ADDR + LCR), lcr);
}

void hal_uart_set_stop_bit(unsigned char bit_num)
{
    unsigned char lcr = READ_BYTE(UART0_BASE_ADDR + LCR);
    if (bit_num == 0) {
        CLEAN_BIT(lcr, STOP_BITS);
    } else if (bit_num == 1) {
        SET_BIT(lcr, STOP_BITS);
    } else {
        return;
    }
    WRITE_BYTE((UART0_BASE_ADDR + LCR), lcr);
}

void hal_uart_set_data_len(unsigned char data_len)
{
    unsigned char lcr = READ_BYTE(UART0_BASE_ADDR + LCR);
    CLEAN_BIT(lcr, WORD_LEN0);
    CLEAN_BIT(lcr, WORD_LEN1);
    switch (data_len) {
        case 6:
            SET_BIT(lcr, WORD_LEN0);
            break;
        case 7:
            SET_BIT(lcr, WORD_LEN1);
            break;
        case 8:
            SET_BIT(lcr, WORD_LEN0);
            SET_BIT(lcr, WORD_LEN1);
            break;
        case 5:
        default:
            break;
    }
    WRITE_BYTE((UART0_BASE_ADDR + LCR), lcr);
}

void hal_uart_putc(unsigned char ch)
{
    while ((READ_BYTE(UART0_BASE_ADDR + LSR) & LSR_TX_IDLE) == 0);
	WRITE_BYTE((UART0_BASE_ADDR + THR), ch);
}

void hal_uart_write(const char *s)
{
	while (*s) {
		hal_uart_putc(*s++);
	}
}