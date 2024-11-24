#ifndef __BESTFIT_H__
#define __BESTFIT_H__

#include "ztype.h"
#include "dlink.h"

typedef struct {
    DLINK_NODE node_list;
    UINT32  size_and_used;
    union {
        struct {
            UINT32 task_id : 8;
            UINT32 magic_num : 24;
        };
        UINT32 rsv_for_free;
    };
} MEM_NODE_INFO;

typedef struct {
    DLINK_NODE head_list;
    MEM_NODE_INFO *first_node;
} MEM_HEAD_INFO;

typedef struct {
    DLINK_NODE pool_list;
    MEM_HEAD_INFO *head;
} MEM_POOL_INFO;

#define MEM_POOL_INFO_SIZE      sizeof(MEM_POOL_INFO)
#define MEM_HEAD_INFO_SIZE      sizeof(MEM_HEAD_INFO)
#define MEM_NODE_INFO_SIZE      sizeof(MEM_NODE_INFO)
#define MEM_SIZE_MASK           (0x7FFFFFFF)
#define MEM_USED_MASK           (0x80000000)
#define MEM_INVALID_SIZE        (0x0)

#define MEM_NODE_GET_SIZE(node)         ((node)->size_and_used & MEM_SIZE_MASK)
#define MEM_NODE_SET_SIZE(node, size)   ((node)->size_and_used = (MEM_NODE_GET_USED((node)) | ((size) & MEM_SIZE_MASK)))
#define MEM_NODE_GET_USED(node)         ((node)->size_and_used & MEM_USED_MASK)
#define MEM_NODE_SET_USED(node)         ((node)->size_and_used |= MEM_USED_MASK)
#define MEM_NODE_CLR_USED(node)         ((node)->size_and_used &= (~MEM_USED_MASK))
#define MEM_NODE_GET_DATA(node)         ((UINT32)((char *)node + MEM_NODE_INFO_SIZE))

VOID os_mem_init(VOID *pool);
MEM_NODE_INFO *os_mem_split_node(MEM_NODE_INFO *mem_node, UINT32 submem_size);
UINTPTR os_mem_get_next_bestfit_free(UINT32 size);
VOID os_mem_set_addr_to_free(VOID *mem_addr);
VOID os_mem_set_used(VOID *mem_addr);

#endif