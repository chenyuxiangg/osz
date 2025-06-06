#include "hal_clint_timer.h"
#include "platform.h"
#include "timer.h"
#include "interrupt.h"
#include "task.h"
#include "softtimer.h"

STATIC VOID timer_int_handle(VOID *args)
{
    os_update_task();
    os_update_swtmr();
    drv_timer_reload(0, CYCLES_PER_TICK);
}

VOID drv_timer_init(UINT8 timer_id)
{
    (void)timer_id;
    if (g_hal_timer_funcs.init != NULL) {
        g_hal_timer_funcs.init();
    }
    osz_interrupt_regist(TIMER_INT_NUM, (interrupt_callback)timer_int_handle);
    drv_timer_reload(0, CYCLES_PER_TICK);
    osz_interrupt_set_pri(TIMER_INT_NUM, 7);
    osz_interrupt_enable(TIMER_INT_NUM);
    drv_timer_enable(0);
    drv_timer_start(0);
}
MODULE_INIT(drv_timer_init, l3)

UINT32 drv_timer_start(UINT8 timer_id)
{
    (void)timer_id;
    UINT32 ret = OS_OK;
    if (g_hal_timer_funcs.start != NULL) {
        ret = g_hal_timer_funcs.start();
    }
    return ret;
}

UINT32 drv_timer_enable(UINT8 timer_id)
{
    (void)timer_id;
    UINT32 ret = OS_OK;
    if (g_hal_timer_funcs.enable != NULL) {
        ret = g_hal_timer_funcs.enable();
    }
    return ret;
}

UINT32 drv_timer_reload(UINT8 timer_id, UINT64 value)
{
    (void)timer_id;
    UINT32 ret = OS_OK;
    if (g_hal_timer_funcs.reload != NULL) {
        ret = g_hal_timer_funcs.reload(value);
    }
    return ret;
}

UINT32 drv_timer_disable(UINT8 timer_id)
{
    (void)timer_id;
    UINT32 ret = OS_OK;
    if (g_hal_timer_funcs.disable != NULL) {
        ret = g_hal_timer_funcs.disable();
    }
    return ret;
}

UINT32 drv_timer_get_time_stamp(UINT64 *us)
{
    UINT32 ret = OS_OK;
    if (g_hal_timer_funcs.get_timer_stamp != NULL) {
        ret = g_hal_timer_funcs.get_timer_stamp(us);
    }
    return ret;
}