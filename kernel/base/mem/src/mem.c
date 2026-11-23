#include "comm.h"
#include "platform.h"
#include "mem.h"

STATIC UINT32 inner_heap_size(VOID)
{
    return (UINT32)((UINT8 *)DEFAULT_HEAP_END - (UINT8 *)DEFAULT_HEAP_START);
}

VOID osz_memory_init(VOID)
{
    (VOID)inner_mem_init((VOID *)DEFAULT_HEAP_START, inner_heap_size());
}
MODULE_INIT(osz_memory_init, l1)

VOID *osz_malloc(UINT32 size)
{
    VOID *addr = NULL;

    if (inner_mem_alloc(size, &addr) != OS_OK) {
        return NULL;
    }
    return addr;
}

VOID *osz_zalloc(UINT32 size)
{
    VOID *addr = osz_malloc(size);

    if (addr == NULL) {
        return NULL;
    }
    memset(addr, 0, size);
    return addr;
}

VOID osz_free(VOID *mem)
{
    /* NULL 直接返回；错误码只用于内部定位，不改变对外 VOID 契约 */
    (VOID)inner_mem_free(mem);
}
