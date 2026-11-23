#ifndef __OSZ_MEM_H__
#define __OSZ_MEM_H__

#include "platform.h"
#include "bestfit.h"

/*
 * 单次申请上限不再用编译期常量：堆区间在 inner_mem_init 时才能确定，
 * 由 inner_mem_alloc 按实际区间校验（见 bestfit.c inner_max_alloc）。
 */

VOID osz_memory_init(VOID);
VOID *osz_malloc(UINT32 size);
VOID *osz_zalloc(UINT32 size);
VOID osz_free(VOID *mem);

#endif
