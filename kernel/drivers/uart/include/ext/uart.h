#ifndef __UART_H__
#define __UART_H__

#define UART_IRQ_DMA_TX     (7)
#define UART_IRQ_DMA_RX     (6)
#define UART_IRQ_MS         (3)
#define UART_IRQ_RLS        (2)
#define UART_IRQ_THRE       (1)
#define UART_IRQ_DATA_READY (0)
#define UART_BAUDRATE       (9600)
#define UART_DATA_LEN_BASE  (5)

typedef enum {
    UART_DL_5 = 0,
    UART_DL_6 = 1,
    UART_DL_7 = 2,
    UART_DL_8 = 3
} UART_DATA_LENGTH;

typedef struct {
    unsigned char parity_en : 1;
    unsigned char parity_mode : 1;
    unsigned char parity_force_en : 1;
    unsigned char stop_bit : 1;
    unsigned char data_len : 2;
    unsigned char rsv : 2;
} UART_PARAMS;

void drv_uart_init(void);
void drv_uart_set_params(const UART_PARAMS params);
void drv_uart_write(const char *str);
void hal_uart_set_data_len(unsigned char data_len);

#endif