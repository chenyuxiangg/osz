#include "comm.h"
#include "hal_clint_timer.h"
#include "timer.h"
#include "interrupt.h"
#include "schedule.h"
#include "task.h"

STATIC VOID inner_tick_int_handle(VOID *args)
{
    os_update_task();
    drv_timer_reload(0, TIMER_CYCLES_PER_TICK);
}

STATIC VOID inner_tick_init(VOID)
{
    drv_timer_init(0);
    osz_interrupt_regist(TIMER_INT_NUM, (interrupt_callback)inner_tick_int_handle);
    drv_timer_reload(0, TIMER_CYCLES_PER_TICK);
    osz_interrupt_set_pri(TIMER_INT_NUM, 7);
    osz_interrupt_enable(TIMER_INT_NUM);
    drv_timer_enable(0);
    drv_timer_start(0);
}
MODULE_INIT(inner_tick_init, l3)
