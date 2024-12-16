#ifndef __ZOS_MEM_H__
#define __ZOS_MEM_H__

#include "platform.h"
#include "bestfit.h"

#define MEM_VALID_MAX_SIZE      (DEFAULT_HEAP_SIZE - MEM_POOL_INFO_SIZE - MEM_HEAD_INFO_SIZE)

VOID osz_memory_init(VOID *pool);
VOID *osz_malloc(UINT32 size);
VOID *osz_zalloc(UINT32 size);
VOID osz_free(VOID *mem);

#endif