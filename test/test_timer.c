#include "comm.h"
#include "timer.h"

void test_timer(void)
{
    drv_timer_init(0);
    drv_timer_enable(0);
    drv_timer_start(0);
    UINT64 us = 0x12345678;
    printf("time stamp1: %lld\n", us);
    drv_timer_get_time_stamp(&us);
    printf("time stamp1: %lld\n", us);
    drv_timer_get_time_stamp(&us);
    printf("time stamp2: %lld\n", us);
}