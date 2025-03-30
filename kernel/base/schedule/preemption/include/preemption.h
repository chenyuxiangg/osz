#ifndef __PREEMPTION_H__
#define __PREEMPTION_H__
#include "comm.h"
#include "arch_adapter.h"

#define SCHEDULE_LOCK(pintSave) do { \
    ARCH_INT_LOCK(*pintSave); \
} while(0)

#define SCHEDULE_UNLOCK(intSave) do { \
    ARCH_INT_UNLOCK(intSave); \
} while(0)

VOID os_preemp_sched_init(VOID);
VOID os_preemp_schedule_with_pm(VOID);
VOID os_preemp_reschedule(VOID);

#endif