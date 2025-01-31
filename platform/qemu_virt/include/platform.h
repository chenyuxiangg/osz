#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/* ======== Register Operat ========*/
#define READ_BYTE(addr)             ({unsigned char val = *(volatile unsigned char *)(addr);val;})
#define WRITE_BYTE(addr, val)       (*(volatile unsigned char *)(addr)=val)
#define CLEAN_BIT(val, bit)         ({val=(~(1 << bit)) & val;val;})
#define SET_BIT(val, bit)           ({val=((1 << bit) | val);val;})

/* ======== UART ======== */
#define UART0_CLOCK             (3686400)
#define UART0_BASE_ADDR         (0x10000000)

/* ======== PLIC ======== */
#define PLIC_BASE_ADDR          (0xC000000)
#define PLIC_HART0_MACHINE      (0x0)

/* ======== PLIC ======== */
#define CLINT_BASE_ADDR         (0x200000)

/* ======== System ======== */
extern unsigned int  __system_heap_start;
extern unsigned int  __system_heap_size;
extern unsigned int  __system_heap_end;
#define DEFAULT_HEAP_START      (&__system_heap_start)
#define DEFAULT_HEAP_SIZE       (&__system_heap_size)
#define DEFAULT_HEAP_END        (&__system_heap_end)

#endif