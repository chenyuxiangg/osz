#ifndef __CLINT_H__
#define __CLINT_H__
#include "comm.h"
#include "clint_errno.h"

#define CLINT_MTIMER_BASE_OFFSET        (0xBFF8)
#define CLINT_MTIMECMP_BASE_OFFSET      (0x4000)
#define CLINT_MSIP_BASE_OFFSET          (0x0)

#define CLINT_GET_MTIMER(base, val) \
do { \
    (val) = *(volatile UINT64 *)((base) + CLINT_MTIMER_BASE_OFFSET); \
} while(0);

// Set MTIMECMP[hart] to val. 
// To prevent mtime interrupts from being triggered by mistake, first configure mtimecmp low 32bit to -1
#define CLINT_SET_MTIMECPM(base, hart, val) \
do { \
    UINT32 low = (val) & 0xFFFFFFFF; \
    UINT32 high = (val) >> 32; \
    *(volatile UINT32 *)((base) + CLINT_MTIMECMP_BASE_OFFSET + 0x8 * (hart)) = -1; \
    *(volatile UINT32 *)((base) + CLINT_MTIMECMP_BASE_OFFSET + 0x8 * (hart)) = low; \
    *(volatile UINT32 *)((base) + CLINT_MTIMECMP_BASE_OFFSET + 0x8 * (hart) + 0x4) = high; \
} while(0);

#define CLINT_PEND_MSIP(base, hart) \
do { \
    *(volatile UINT32 *)((base) + CLINT_MSIP_BASE_OFFSET + 0x4 * (hart)) = 0x1; \
} while(0);

#define CLINT_CLEAN_MSIP(base) \
do { \
    *(volatile UINT32 *)((base) + CLINT_MSIP_BASE_OFFSET + 0x4 * (hart)) = 0x0; \
} while(0);

#endif