#ifndef __HAL_CLINT_TIMER_H__
#define __HAL_CLINT_TIMER_H__
#include "hal_timer_comm.h"
#include "clint.h"              // 由于clint中包含了mtime的内存映射，所以包含这个头文件

#define TIMER_BASE_ADDR         (0x2000000)
#define TIMER_INT_NUM           (7)
#define TIMER_CYCLES_PER_US     (10)
#define TIMER_100_MS            (100 * 1000)
#define TIMER_CYCLES_PER_TICK   (TIMER_100_MS * TIMER_CYCLES_PER_US)
#endif