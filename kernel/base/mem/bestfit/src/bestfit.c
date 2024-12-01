#include "comm.h"
#include "platform.h"
#include "bestfit.h"

MEM_POOL_INFO *g_mempool = NULL;

STATIC MEM_NODE_INFO *os_get_end_mem_node()
{
    MEM_NODE_INFO *end_node = (MEM_NODE_INFO *)((UINT8 *)DEFAULT_HEAP_END - MEM_NODE_INFO_SIZE);
    return end_node;
}

STATIC VOID os_mempool_init(VOID *pool)
{
    ASSERT(pool);
    if (g_mempool != NULL) {
        return;
    }

    g_mempool = (MEM_POOL_INFO *)pool;
    g_mempool->head = (MEM_HEAD_INFO *)((UINT8 *)g_mempool + MEM_POOL_INFO_SIZE);
    dlink_init(&(g_mempool->pool_list));
}

STATIC VOID os_memhead_init(MEM_HEAD_INFO * head)
{
    ASSERT(head);
    dlink_init(&(head->head_list));
    head->first_node = (MEM_NODE_INFO *)((char *)head + MEM_HEAD_INFO_SIZE);
}

STATIC BOOL os_is_bestfit_free(MEM_NODE_INFO *free_node, UINT32 size)
{
    UINT32 expect_size = ALIGN_UP(size, 0x10);
    if (MEM_NODE_GET_SIZE(free_node) == expect_size) {
        return TRUE;
    }
    return FALSE;
}

STATIC VOID os_mem_node_keep_sort(MEM_NODE_INFO *node)
{
    MEM_NODE_INFO *end_node = os_get_end_mem_node();
    DLINK_NODE *iter = NULL;
    DLINK_NODE *last_sort_node = NULL;
    BOOL node_enqueue_flag = FALSE;
    DLINK_FOREACH(iter, &(end_node->node_list)) {
        MEM_NODE_INFO *entry = DLINK_ENTRY(MEM_NODE_INFO, node_list, iter);
        if (MEM_NODE_GET_SIZE(entry) == MEM_INVALID_SIZE) {
            continue;
        } else if (MEM_NODE_GET_SIZE(entry) >= MEM_NODE_GET_SIZE(node)) {
            last_sort_node = iter;
            continue;
        } else {
            node_enqueue_flag = TRUE;
            dlink_insert_tail(iter, &(node->node_list));
            break;
        }
    }
    if (node_enqueue_flag != TRUE) {
        dlink_insert_head(last_sort_node, &(node->node_list));
    }
}

VOID os_mem_init(VOID *pool)
{
    ASSERT(pool);
    os_mempool_init(pool);
    os_memhead_init(g_mempool->head);

    MEM_NODE_INFO *first_node = g_mempool->head->first_node;
    dlink_init(&(first_node->node_list));
    UINT32 mem_size = (UINT32)DEFAULT_HEAP_SIZE - MEM_POOL_INFO_SIZE - MEM_HEAD_INFO_SIZE - MEM_NODE_INFO_SIZE * 2;
    first_node->size_and_used = (MEM_SIZE_MASK & mem_size);; // two node: first node and end node at init stage.
    first_node->rsv_for_free = 0;       // rsv_for_free field not used this time.

    MEM_NODE_INFO *end_node = (MEM_NODE_INFO *)((char *)DEFAULT_HEAP_END - MEM_NODE_INFO_SIZE);
    end_node->rsv_for_free = 0;
    MEM_NODE_SET_USED(end_node);
    MEM_NODE_SET_SIZE(end_node, MEM_INVALID_SIZE);
    dlink_insert_tail(&(first_node->node_list), &(end_node->node_list));
}

MEM_NODE_INFO *os_mem_split_node(MEM_NODE_INFO *mem_node, UINT32 submem_size)
{
    ASSERT(mem_node);
    if (submem_size == MEM_NODE_GET_SIZE(mem_node)) {
        return mem_node;
    } else if (submem_size + MEM_NODE_INFO_SIZE > MEM_NODE_GET_SIZE(mem_node)) {
        return NULL;
    }
    MEM_NODE_INFO *new_node = (MEM_NODE_INFO *)((UINT8 *)mem_node + MEM_NODE_GET_SIZE(mem_node) - submem_size);
    MEM_NODE_SET_SIZE(mem_node, MEM_NODE_GET_SIZE(mem_node) - MEM_NODE_INFO_SIZE - submem_size);
    new_node->rsv_for_free = 0;
    os_mem_node_keep_sort(new_node);
    return new_node;
}

UINTPTR os_mem_get_next_bestfit_free(UINT32 size)
{
    MEM_NODE_INFO *end_node = os_get_end_mem_node();
    MEM_NODE_INFO *last_free_node = NULL;
    MEM_NODE_INFO *bestfit_free_node = NULL;
    DLINK_NODE *iter = NULL;
    DLINK_FOREACH(iter, &(end_node->node_list)) {
        MEM_NODE_INFO *entry = DLINK_ENTRY(MEM_NODE_INFO, node_list, iter);
        if (MEM_NODE_GET_USED(entry) & MEM_USED_MASK) {
            continue;
        } else if (os_is_bestfit_free(entry, size) == FALSE) {
            last_free_node = entry;
            continue;
        } else {
            bestfit_free_node = entry;
            break;
        }
    }
    if ((bestfit_free_node == NULL) && (last_free_node == NULL)) {
        return NULL;
    }
    if (bestfit_free_node != NULL) {
        return MEM_NODE_GET_DATA(bestfit_free_node);
    }
    bestfit_free_node = os_mem_split_node(last_free_node, ALIGN_UP(size, 0x10));
    MEM_NODE_SET_SIZE(bestfit_free_node, ALIGN_UP(size, 0x10));
    return MEM_NODE_GET_DATA(bestfit_free_node);
}

VOID os_mem_set_used(VOID *mem_addr)
{
    ASSERT(mem_addr);
    MEM_NODE_INFO *node = (MEM_NODE_INFO *)((char *)mem_addr - MEM_NODE_INFO_SIZE);
    MEM_NODE_SET_USED(node);
}

VOID os_mem_set_addr_to_free(VOID *mem_addr)
{
    ASSERT(mem_addr);
    MEM_NODE_INFO *node = (MEM_NODE_INFO *)((char *)mem_addr - MEM_NODE_INFO_SIZE);
    MEM_NODE_CLR_USED(node);
}