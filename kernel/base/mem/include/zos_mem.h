#ifndef __ZOS_MEM_H__
#define __ZOS_MEM_H__

#include "platform.h"
#include "bestfit.h"

#define MEM_VALID_MAX_SIZE      (DEFAULT_HEAP_SIZE - MEM_POOL_INFO_SIZE - MEM_HEAD_INFO_SIZE)

VOID zos_memory_init(VOID *pool);
VOID *zos_malloc(UINT32 size);
VOID *zos_zalloc(UINT32 size);
VOID zos_free(VOID *mem);

#endif