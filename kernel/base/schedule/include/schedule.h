#ifndef __OSZ_SCHED_H__
#define __OSZ_SCHED_H__
#include "comm.h"

extern VOID first_schedule(VOID);
extern VOID schedule(VOID);
#ifdef OSZ_CFG_COORDINATION
#include "coordination.h"
#define os_schedule_init    os_coord_sched_init
#define os_schedule         os_coord_schedule
#endif

#ifdef OSZ_CFG_PREEMPTION
#include "preemption.h"
#define os_schedule_init    os_preemp_sched_init
#define os_schedule         os_preemp_reschedule
extern BOOL g_need_preemp;
extern BOOL g_int_active;
#endif

#endif