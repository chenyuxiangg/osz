#include "comm.h"
#include "interrupt.h"
#include "csr.h"

#define UART0_BASE_ADDR                             0x10000000
#define LSR     0x5
#define LSR_RX_VALID     (1 << 0)
#define RHR     0x0

static void uart_interrupt_recv(void *args)
{
    if ((READ_BYTE(UART0_BASE_ADDR + LSR) & LSR_RX_VALID) == 0) {
        return;
    }
	UINT8 ch = READ_BYTE(UART0_BASE_ADDR + RHR);
    printf("%c", ch);
}

void test_interrupt(void)
{
    osz_interrupt_init();
    osz_interrupt_regist(0x1a, (interrupt_callback)uart_interrupt_recv);
    osz_interrupt_enable(0x1a);
    osz_interrupt_set_pri(0x1a, 1);
    osz_interrupt_set_pri_th(0);
    CSR_WRITE(mstatus, 0x8);
    CSR_WRITE(mie, (1 << 11));
    UINT32 mstatus_v = CSR_READ(mstatus);
    UINT32 mie_v = CSR_READ(mie);
    printf("mstatus: %#x\n", mstatus_v);
    printf("mie: %#x\n", mie_v);
}