#include "comm.h"
#include "mem.h"
#include "uart.h"

static void test_mem_operator()
{
    int *arr = (int *)osz_malloc(sizeof(int) * 4);
    printf("arr addr: 0x%x\n", arr);
    int arr1[] = { 0xa, 0xb, 0xc, 0xd };
    for (int i = 0; i < sizeof(arr); ++i) {
        arr[i] = i * i;
    }
    for (int i = 0; i < sizeof(arr); ++i) {
        printf("index[%d]: %d\n", i, arr[i]);
    }
    printf("========= memset ===========\n");
    memset(arr, 0, sizeof(arr) * sizeof(int));
    for (int i = 0; i < sizeof(arr); ++i) {
        printf("index[%d]: %d\n", i, arr[i]);
    }
    printf("========= memcpy ===========\n");
    memcpy(arr, arr1, sizeof(arr) * sizeof(int));
    for (int i = 0; i < sizeof(arr); ++i) {
        printf("index[%d]: %d\n", i, arr[i]);
    }
    osz_free(arr);
}

void test_mem(void)
{
    osz_memory_init((void *)DEFAULT_HEAP_START);
    char *start = (char *)DEFAULT_HEAP_START;
    char *arr1 = (char *)osz_malloc(3);
    char *arr2 = (char *)osz_malloc(16);
    char *expect1 = start + (UINT32)DEFAULT_HEAP_SIZE - MEM_NODE_INFO_SIZE - 0x10;
    char *expect2 = expect1 - MEM_NODE_INFO_SIZE - 0x10;
    if (arr1 == expect1 && arr2 == expect2) {
        drv_uart_write("[mem] test alloc success!\n");
    } else {
        drv_uart_write("[mem] test alloc fail!\n");
        return;
    }

    osz_free(arr1);
    osz_free(arr2);
    arr1 = NULL;
    arr2 = NULL;
    char *arr3 = (char *)osz_malloc(16);
    char *arr4 = (char *)osz_malloc(3);
    if (arr4 == expect2 && arr3 == expect1) {
        drv_uart_write("[mem] test alloc success!\n");
    } else {
        drv_uart_write("[mem] test alloc fail!\n");
        return;
    }
    osz_free(arr3);
    osz_free(arr4);
    test_mem_operator();
}