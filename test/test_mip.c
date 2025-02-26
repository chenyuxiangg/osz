#include "comm.h"

typedef unsigned int reg_t;
static inline reg_t r_mip()
{
	reg_t x;
	asm volatile("csrr %0, mip" : "=r" (x));
	return x;
}

static inline void w_mip(reg_t x)
{
	asm volatile("csrw mip, %0" : : "r" (x));
}

/*
* MEIP只读，且写不会报错
*/
void test_mip(void)
{
    printf("before mip: %#x\n", r_mip());
    w_mip(r_mip() | (1 < 11));
    printf("after mip: %#x\n", r_mip());
}