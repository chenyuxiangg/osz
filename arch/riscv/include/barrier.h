#ifndef __BARRIER_H__
#define __BARRIER_H__
#include "comm.h"

#define wfi() do { \
    __asm__ __volatile__("wfi"); \
} while(0)

#endif