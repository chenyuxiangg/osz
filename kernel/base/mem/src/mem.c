#include "comm.h"
#include "bestfit.h"
#include "mem.h"

VOID osz_memory_init(VOID *pool)
{
    os_mem_init(pool);
}

VOID *osz_malloc(UINT32 size)
{
    if (size <= 0 || size >= (UINT32)MEM_VALID_MAX_SIZE) {
        return NULL;
    }
    VOID *mem_addr = (VOID *)os_mem_get_next_bestfit_free(size);
    os_mem_set_used(mem_addr);
    return mem_addr;
}

VOID *osz_zalloc(UINT32 size)
{
    if (size <= 0 || size >= (UINT32)MEM_VALID_MAX_SIZE) {
        return NULL;
    }
    
    VOID *addr = (VOID *)os_mem_get_next_bestfit_free(size);
    // memset(addr, 0, size);
    return addr;
}

VOID osz_free(VOID *mem)
{
    ASSERT(mem);
    os_mem_set_addr_to_free(mem);
}