#ifndef __HAL_TIMER_COMM_H__
#define __HAL_TIMER_COMM_H__
#include "comm.h"

typedef struct {
    UINT32 (*init)(VOID);
    UINT32 (*start)(VOID);
    UINT32 (*enable)(VOID);
    UINT32 (*disable)(VOID);
    UINT32 (*reload)(UINT64 value);
    UINT32 (*get_timer_stamp)(UINT64 *us);
} HAL_TIMER_FUNCTIONS;
extern HAL_TIMER_FUNCTIONS g_hal_timer_funcs;

#endif