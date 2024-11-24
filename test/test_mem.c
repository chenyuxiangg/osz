#include "zos_mem.h"
#include "uart.h"

void test_mem(void)
{
    zos_memory_init((void *)DEFAULT_HEAP_START);
    char *start = (char *)DEFAULT_HEAP_START;
    char *arr1 = (char *)zos_malloc(3);
    char *arr2 = (char *)zos_malloc(16);
    char *expect1 = start + (UINT32)DEFAULT_HEAP_SIZE - MEM_NODE_INFO_SIZE - 0x10;
    char *expect2 = expect1 - MEM_NODE_INFO_SIZE - 0x10;
    if (arr1 == expect1 && arr2 == expect2) {
        drv_uart_write("[mem] test alloc success!\n");
    } else {
        drv_uart_write("[mem] test alloc fail!\n");
        return;
    }

    zos_free(arr1);
    zos_free(arr2);
    arr1 = NULL;
    arr2 = NULL;
    char *arr3 = (char *)zos_malloc(16);
    char *arr4 = (char *)zos_malloc(3);
    if (arr4 == expect2 && arr3 == expect1) {
        drv_uart_write("[mem] test alloc success!\n");
    } else {
        drv_uart_write("[mem] test alloc fail!\n");
        return;
    }
}