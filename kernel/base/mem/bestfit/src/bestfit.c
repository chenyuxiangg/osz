#include "assert.h"
#include "align.h"
#include "qualifer.h"
#include "ztype.h"
#include "bestfit.h"

#if defined(OSZ_TESTING)
#include "printf.h"     /* 仅宿主单测：自检失败时打印原因便于定位 */
#define MEM_DBG_PRINT(...)      printf(__VA_ARGS__)
#else
#define MEM_DBG_PRINT(...)      do { } while (0)
#endif

#if !defined(OSZ_TESTING)
#include "arch_adapter.h"
#define MEM_INT_LOCK(save)          ARCH_INT_LOCK(save)
#define MEM_INT_UNLOCK(save)        ARCH_INT_UNLOCK(save)
#else
/* 宿主单测为单线程环境：临界区退化为空操作，被测算术路径保持一致 */
#define MEM_INT_LOCK(save)          do { (save) = 0; } while (0)
#define MEM_INT_UNLOCK(save)        do { } while (0)
#endif

STATIC MEM_POOL_INFO *g_mempool = NULL;
STATIC UINT8 *g_heap_start = NULL;
STATIC UINT8 *g_heap_end = NULL;

STATIC MEM_NODE_INFO *inner_end_node(VOID)
{
    return (MEM_NODE_INFO *)((UINT8 *)g_heap_end - MEM_NODE_INFO_SIZE);
}

STATIC MEM_NODE_INFO *inner_first_node(VOID)
{
    return g_mempool->head->first_node;
}

STATIC UINT32 inner_max_alloc(VOID)
{
    return (UINT32)(g_heap_end - g_heap_start)
           - MEM_POOL_INFO_SIZE - MEM_HEAD_INFO_SIZE - 2 * MEM_NODE_INFO_SIZE;
}

/* ---- 空闲链维护：只挂真空闲块，size 严格升序 ---- */
STATIC VOID inner_free_list_insert(MEM_NODE_INFO *node)
{
    DLINK_NODE *pos = g_mempool->head->free_list.next;

    while (pos != &(g_mempool->head->free_list)) {
        MEM_NODE_INFO *cur = STRUCT_ENTRY(MEM_NODE_INFO, node_list, pos);
        if (MEM_NODE_GET_SIZE(cur) > MEM_NODE_GET_SIZE(node)) {
            break;
        }
        pos = pos->next;
    }
    dlink_insert_head(pos, &(node->node_list));
}

STATIC VOID inner_free_list_remove(MEM_NODE_INFO *node)
{
    dlink_del_node(&(node->node_list));
}

STATIC MEM_NODE_INFO *inner_best_fit(UINT32 need)
{
    DLINK_NODE *pos = NULL;

    DLINK_FOREACH(pos, &(g_mempool->head->free_list)) {
        MEM_NODE_INFO *cur = STRUCT_ENTRY(MEM_NODE_INFO, node_list, pos);
        if (MEM_NODE_GET_SIZE(cur) >= need) {
            return cur;     /* 升序 ⇒ 首个满足者即 best-fit */
        }
    }
    return NULL;
}

#if MEM_FEATURE_DEBUG
STATIC VOID inner_panic(const CHAR *reason)
{
    /* DEBUG 配置下内部一致性被破坏：先打印原因便于定位，再停机 */
    MEM_DBG_PRINT("[MEM CHECK] %s\n", reason);
    ASSERT(NULL);
}

/*
 * 全遍历自检：物理链首尾相接、对齐、size 粒度、prev_size 与 magic、
 * 相邻块不同时空闲（I7）、空闲链有序且节点数与物理空闲块数一致。
 * 仅 DEBUG 配置参与编译，不改变功能路径。
 */
STATIC VOID inner_self_check(VOID)
{
    MEM_HEAD_INFO *head = g_mempool->head;
    MEM_NODE_INFO *end = inner_end_node();
    MEM_NODE_INFO *first = inner_first_node();
    MEM_NODE_INFO *cur = first;
    UINT32 guard = 0;
    UINT32 max_nodes = (UINT32)(g_heap_end - g_heap_start) / (MEM_NODE_INFO_SIZE + MEM_ALIGN) + 4U;
    UINT32 free_blocks = 0;
    UINT32 free_nodes = 0;
    MEM_NODE_INFO *last = NULL;
    DLINK_NODE *pos = NULL;

    while (cur != end) {
        if (++guard > max_nodes) {
            inner_panic("walk overrun");
        }
        if ((((UINTPTR)cur) % MEM_ALIGN) != 0) {
            inner_panic("block not aligned");
        }
        if ((MEM_NODE_GET_SIZE(cur) % MEM_ALIGN) != 0) {
            inner_panic("size not aligned");
        }
        if ((cur->prev_size & MEM_PREV_MAGIC_MASK) != MEM_PREV_MAGIC) {
            inner_panic("prev_size magic lost");
        }
        if (cur == first) {
            if (MEM_NODE_GET_PREV(cur) != 0) {
                inner_panic("first node has prev");
            }
        } else {
            MEM_NODE_INFO *prev = MEM_NODE_PREV(cur);
            if (MEM_NODE_GET_SIZE(prev) != MEM_NODE_GET_PREV(cur)) {
                inner_panic("prev_size mismatch");
            }
            if (MEM_NODE_NEXT(prev) != cur) {
                inner_panic("physical chain broken");
            }
        }
        if (MEM_NODE_GET_USED(cur) == 0) {
            MEM_NODE_INFO *next_blk = MEM_NODE_NEXT(cur);
            free_blocks++;
            if (MEM_NODE_GET_SIZE(cur) < MEM_MIN_SPLIT) {
                inner_panic("free block below min split");
            }
            if ((next_blk != end) && (MEM_NODE_GET_USED(next_blk) == 0)) {
                inner_panic("I7: adjacent free blocks");
            }
        }
        cur = MEM_NODE_NEXT(cur);
    }
    if (cur != end) {
        inner_panic("walk did not reach end");
    }
    if ((MEM_NODE_GET_USED(end) == 0) || (MEM_NODE_GET_SIZE(end) != MEM_INVALID_SIZE)) {
        inner_panic("end node malformed");
    }

    DLINK_FOREACH(pos, &(head->free_list)) {
        MEM_NODE_INFO *node = STRUCT_ENTRY(MEM_NODE_INFO, node_list, pos);
        if (MEM_NODE_GET_USED(node) != 0) {
            inner_panic("free list holds used block");
        }
        if ((last != NULL) && (MEM_NODE_GET_SIZE(last) > MEM_NODE_GET_SIZE(node))) {
            inner_panic("free list not ascending");
        }
        last = node;
        free_nodes++;
        if (free_nodes > free_blocks) {
            inner_panic("free list duplicate/cycle");
        }
    }
    if (free_nodes != free_blocks) {
        inner_panic("free list count mismatch");
    }
}
#else
#define inner_self_check()          do { } while (0)
#endif

/* 地址合法性校验：两种配置都执行，返回错误码而不停机 */
STATIC UINT32 inner_validate_ptr(VOID *ptr, MEM_NODE_INFO **out_node)
{
    UINTPTR addr = (UINTPTR)ptr;
    MEM_NODE_INFO *node = NULL;

    *out_node = NULL;
    if (ptr == NULL) {
        return MEM_IS_NULL_ERR;
    }
    if (((addr % MEM_ALIGN) != 0) || (addr < MEM_NODE_INFO_SIZE)) {
        return MEM_NODE_CORRUPTED_ERR;
    }
    node = (MEM_NODE_INFO *)((UINT8 *)ptr - MEM_NODE_INFO_SIZE);
    if (((UINT8 *)node < (UINT8 *)inner_first_node()) ||
        ((UINT8 *)node >= (UINT8 *)inner_end_node())) {
        return MEM_BOUNDARY_OVERFLOW_ERR;
    }
    *out_node = node;
    return OS_OK;
}

UINT32 inner_mem_init(VOID *pool, UINT32 pool_size)
{
    MEM_HEAD_INFO *head = NULL;
    MEM_NODE_INFO *first_node = NULL;
    MEM_NODE_INFO *end_node = NULL;
    UINT32 data_size = 0;

    if (pool == NULL) {
        return MEM_IS_NULL_ERR;
    }
    if ((((UINTPTR)pool) % MEM_ALIGN) != 0) {
        return MEM_SIZE_INVALID_ERR;
    }
    if (pool_size <= (MEM_POOL_INFO_SIZE + MEM_HEAD_INFO_SIZE +
                      2 * MEM_NODE_INFO_SIZE + MEM_MIN_SPLIT)) {
        return MEM_SIZE_INVALID_ERR;
    }
    if ((pool_size % MEM_ALIGN) != 0) {
        pool_size &= ~(MEM_ALIGN - 1U);        /* 数据区末端按 16 B 收敛 */
    }

    g_heap_start = (UINT8 *)pool;
    g_heap_end = (UINT8 *)pool + pool_size;
    g_mempool = (MEM_POOL_INFO *)pool;
    dlink_init(&(g_mempool->pool_list));

    head = (MEM_HEAD_INFO *)((UINT8 *)pool + MEM_POOL_INFO_SIZE);
    dlink_init(&(head->free_list));
    head->first_node = (MEM_NODE_INFO *)((UINT8 *)head + MEM_HEAD_INFO_SIZE);
    g_mempool->head = head;

    first_node = head->first_node;
    data_size = pool_size - MEM_POOL_INFO_SIZE - MEM_HEAD_INFO_SIZE - 2 * MEM_NODE_INFO_SIZE;
    data_size &= ~(MEM_ALIGN - 1U);

    first_node->size_and_used = data_size;      /* used = 0 */
    MEM_NODE_SET_PREV(first_node, 0);
    inner_free_list_insert(first_node);

    end_node = (MEM_NODE_INFO *)((UINT8 *)first_node + MEM_NODE_INFO_SIZE + data_size);
    end_node->size_and_used = MEM_USED_MASK;    /* size = 0, used = 1 */
    MEM_NODE_SET_PREV(end_node, data_size);

    inner_self_check();
    return OS_OK;
}

UINT32 inner_mem_alloc(UINT32 size, VOID **out)
{
    UINT32 need = 0;
    UINT32 int_save = 0;
    MEM_NODE_INFO *blk = NULL;
    MEM_NODE_INFO *rem = NULL;
    MEM_NODE_INFO *next = NULL;

    if (out == NULL) {
        return MEM_IS_NULL_ERR;
    }
    *out = NULL;
    if (size == 0) {
        return MEM_SIZE_INVALID_ERR;
    }
    if ((g_mempool == NULL) || (g_mempool->head == NULL)) {
        return MEM_POOL_NOT_INIT_ERR;
    }
    need = ALIGN_UP(size, MEM_ALIGN);
    if (need < MEM_MIN_SPLIT) {
        need = MEM_MIN_SPLIT;   /* 最小分配块：16 B 块头 + 16 B 数据 */
    }
    if (need > inner_max_alloc()) {
        return MEM_SIZE_INVALID_ERR;
    }

    MEM_INT_LOCK(int_save);
    blk = inner_best_fit(need);
    if (blk == NULL) {
        MEM_INT_UNLOCK(int_save);
        return MEM_NO_FREE_BLK_ERR;
    }

    /* 切分后 rem 的数据区必须不小于 MEM_MIN_SPLIT，否则整块交付：
       size - need 里还包含 rem 的 16 B 块头，不能直接与该阈值比较。 */
    if ((MEM_NODE_GET_SIZE(blk) - need) < (MEM_MIN_SPLIT + MEM_NODE_INFO_SIZE)) {
        inner_free_list_remove(blk);
        MEM_NODE_SET_USED(blk);
    } else {
        /* 低端切分：rem 紧随 blk 之后，位于高地址 */
        rem = (MEM_NODE_INFO *)((UINT8 *)blk + MEM_NODE_INFO_SIZE + need);
        rem->size_and_used = MEM_NODE_GET_SIZE(blk) - MEM_NODE_INFO_SIZE - need;
        MEM_NODE_SET_PREV(rem, need);

        inner_free_list_remove(blk);
        MEM_NODE_SET_SIZE(blk, need);
        MEM_NODE_SET_USED(blk);

        /* 防御性归并：rem 的后邻若空闲，先归一入链，避免破坏 I7 */
        next = MEM_NODE_NEXT(rem);
        if ((next != inner_end_node()) && (MEM_NODE_GET_USED(next) == 0)) {
            inner_free_list_remove(next);
            rem->size_and_used = MEM_NODE_GET_SIZE(rem) + MEM_NODE_INFO_SIZE +
                                 MEM_NODE_GET_SIZE(next);
        }
        MEM_NODE_SET_PREV(MEM_NODE_NEXT(rem), MEM_NODE_GET_SIZE(rem));
        inner_free_list_insert(rem);
    }

    *out = (VOID *)MEM_NODE_GET_DATA(blk);
    inner_self_check();
    MEM_INT_UNLOCK(int_save);
    return OS_OK;
}

UINT32 inner_mem_free(VOID *ptr)
{
    UINT32 int_save = 0;
    UINT32 ret = OS_OK;
    MEM_NODE_INFO *node = NULL;
    MEM_NODE_INFO *next = NULL;

    if ((g_mempool == NULL) || (g_mempool->head == NULL)) {
        return MEM_POOL_NOT_INIT_ERR;
    }

    MEM_INT_LOCK(int_save);
    ret = inner_validate_ptr(ptr, &node);
    if (ret != OS_OK) {
        MEM_INT_UNLOCK(int_save);
        return ret;
    }
    if (MEM_NODE_GET_USED(node) == 0) {
        MEM_INT_UNLOCK(int_save);
        return MEM_DOUBLE_FREE_ERR;
    }

    MEM_NODE_CLR_USED(node);        /* 只改块头，不清零数据区 */

    /* ① 后向合并 */
    next = MEM_NODE_NEXT(node);
    if ((next != inner_end_node()) && (MEM_NODE_GET_USED(next) == 0)) {
        inner_free_list_remove(next);
        MEM_NODE_SET_SIZE(node, MEM_NODE_GET_SIZE(node) + MEM_NODE_INFO_SIZE +
                                 MEM_NODE_GET_SIZE(next));
    }

    /* ② 前向合并：合并后以 prev 为首 */
    if (MEM_NODE_GET_PREV(node) != 0) {
        MEM_NODE_INFO *prev = MEM_NODE_PREV(node);
        if (MEM_NODE_GET_USED(prev) == 0) {
            inner_free_list_remove(prev);
            MEM_NODE_SET_SIZE(prev, MEM_NODE_GET_SIZE(prev) + MEM_NODE_INFO_SIZE +
                                    MEM_NODE_GET_SIZE(node));
            node = prev;
        }
    }

    /* ③ 维护后继边界并统一入链 */
    MEM_NODE_SET_PREV(MEM_NODE_NEXT(node), MEM_NODE_GET_SIZE(node));
    inner_free_list_insert(node);

    inner_self_check();
    MEM_INT_UNLOCK(int_save);
    return OS_OK;
}

/* ---- 只读调试接口：供宿主用例与 Shell 观察堆形态 ---- */
MEM_HEAD_INFO *inner_mem_head(VOID)
{
    return (g_mempool == NULL) ? NULL : g_mempool->head;
}

UINT8 *inner_mem_heap_start(VOID)
{
    return g_heap_start;
}

UINT8 *inner_mem_heap_end(VOID)
{
    return g_heap_end;
}
