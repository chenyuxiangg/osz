#include "comm.h"
#include "bestfit.h"
#include "zos_mem.h"

VOID zos_memory_init(VOID *pool)
{
    os_mem_init(pool);
}

VOID *zos_malloc(UINT32 size)
{
    if (size <= 0 || size >= (UINT32)MEM_VALID_MAX_SIZE) {
        return NULL;
    }
    VOID *mem_addr = (VOID *)os_mem_get_next_bestfit_free(size);
    os_mem_set_used(mem_addr);
    return mem_addr;
}

VOID *zos_zalloc(UINT32 size)
{
    if (size <= 0 || size >= (UINT32)MEM_VALID_MAX_SIZE) {
        return NULL;
    }
    
    VOID *addr = (VOID *)os_mem_get_next_bestfit_free(size);
    // memset(addr, 0, size);
    return addr;
}

VOID zos_free(VOID *mem)
{
    ASSERT(mem);
    os_mem_set_addr_to_free(mem);
}