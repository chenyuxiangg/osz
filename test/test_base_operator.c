#include "printf.h"

void test_base_operator(void)
{
    int a = 6;
    int b = a / 2;
    int c = a * 3;
    int d = c % 5;
    printf("b = %d\n", b);
    printf("c = %d\n", c);
    printf("d = %d\n", d);
}

void test_float_operator(void)
{
    float a = 6.6;
    float b = a - 1;
    float c = a / 2;
    printf("b = %f\n", b);
    printf("c = %f\n", c);
}