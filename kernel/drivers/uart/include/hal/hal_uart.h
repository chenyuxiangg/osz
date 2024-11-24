#ifndef __HAL_UART_H__
#define __HAL_UART_H__

/*
* THR: TX FIFO
* RHR: RX FIFO
* LSR: RX STATUS
* DLL: DL寄存器的低8bit,用于配置波特率
* DLM: DL寄存器的高8bit,用于配置波特率
* PSD: 用于配置波特率
* DLAB: LCR的一个字段，用于将复用关系选择为DLL、DLM
*/
#define THR     0x0
#define RHR     0x0
#define IER     0x1
#define LSR     0x5
#define LCR     0x3
#define DLL     0x0
#define DLM     0x1
#define PSD     0x5

/* ======== Register bit define ======== */
#define DLAB            7
#define EVEN_PARITY     4
#define ENABLE_PARITY   3
#define FORCE_PARITY    5
#define STOP_BITS       2
#define WORD_LEN0       0
#define WORD_LEN1       1

/* ======== FIFO State ======== */
#define LSR_TX_IDLE     (1 << 5)

typedef enum {
    PARITY_ODD = 0,
    PARITY_EVEN
} HAL_PARITY_MODE;

void hal_uart_set_baudrate(unsigned int baudrate);
void hal_uart_set_parity_en(unsigned char en);
void hal_uart_set_parity_mode(HAL_PARITY_MODE mode);
void hal_uart_set_parity_force_en(unsigned char en);
void hal_uart_set_stop_bit(unsigned char bit_num);
void hal_uart_set_data_len(unsigned char data_len);
void hal_uart_write(const char *s);
void hal_uart_set_irq_en(unsigned char en);

#endif