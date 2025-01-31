#include "printf.h"
#define CLINT_BASE      (0x2000000)
#define MTIME           (0xbff8)
#define VAL_MTIME       (*(volatile unsigned int *)(CLINT_BASE + MTIME))

void test_mtime(void)
{
    unsigned int mtime_val = VAL_MTIME;
    unsigned int secs = 10 * 1000 * 1000 * 30;
    while(VAL_MTIME - mtime_val < secs);
    printf("delay 30s.\n");
}