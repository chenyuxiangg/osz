#include "hal_clint_timer.h"
#include "platform.h"
#include "csr.h"

STATIC UINT32 hal_clint_timer_reload(UINT64 us)
{
    UINT64 current_time = 0;
    UINT64 cycles_of_us = us * TIMER_CYCLES_PER_US;
    CLINT_GET_MTIMER(TIMER_BASE_ADDR, current_time);
    CLINT_SET_MTIMECPM(TIMER_BASE_ADDR, PLIC_HART0_MACHINE, current_time + cycles_of_us);
    return OS_OK;
}

STATIC UINT32 hal_clint_timer_enable(VOID)
{
    CSR_WRITE(mie, CSR_READ(mie) | MIE_MTIE);
    return OS_OK;
}

STATIC UINT32 hal_clint_timer_disable(VOID)
{
    CSR_CLEAR(mie, MIE_MTIE);
    return OS_OK;
}

STATIC UINT32 hal_clint_timer_get_time_stamp(UINT64 *us)
{
    hal_clint_timer_disable();
    CLINT_GET_MTIMER(TIMER_BASE_ADDR, *us);
    *us /= TIMER_CYCLES_PER_US;
    hal_clint_timer_enable();
    return OS_OK;
}

HAL_TIMER_FUNCTIONS g_hal_timer_funcs = {
    .enable = hal_clint_timer_enable,
    .disable = hal_clint_timer_disable,
    .reload = hal_clint_timer_reload,
    .get_timer_stamp = hal_clint_timer_get_time_stamp,
};