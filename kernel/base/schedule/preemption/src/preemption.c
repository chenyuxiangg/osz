#include "preemption.h"
#include "schedule.h"
#include "task.h"

UINT32 g_test_sched = 0;

UINT32 is_need_sched(VOID)
{
    return g_test_sched;
}

VOID os_preemp_sched_init(VOID)
{

}

VOID os_preemp_schedule(VOID)
{
    schedule();
}