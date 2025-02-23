#ifndef __TIMER_H__
#define __TIMER_H__
#include "comm.h"

VOID   drv_timer_init(UINT8 timer_id);
UINT32 drv_timer_start(UINT8 timer_id);
UINT32 drv_timer_enable(UINT8 timer_id);
UINT32 drv_timer_reload(UINT8 timer_id, UINT64 value);
UINT32 drv_timer_disable(UINT8 timer_id);
UINT32 drv_timer_get_time_stamp(UINT64 *us);

#endif