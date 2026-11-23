#ifndef __BESTFIT_H__
#define __BESTFIT_H__

#include "ztype.h"
#include "dlink.h"
#include "mem_feature.h"

/*
 * 块头 16 字节（rv32）：
 *   node_list     8 B  空闲时挂空闲链，占用时内容无效
 *   size_and_used 4 B  size[30:0] | used[31]
 *   prev_size     4 B  [31:30] 固定 magic 0b10，[29:0] 前邻数据区大小（0 = 无前邻）
 *
 * prev_size 的 magic 位只用于 DEBUG 配置下的块头校验，不改变结构体布局，
 * 因此两种配置（MEM_FEATURE_DEBUG=0/1）的 ABI 完全一致。
 */
typedef struct {
    DLINK_NODE node_list;
    UINT32  size_and_used;
    UINT32  prev_size;
} MEM_NODE_INFO;

typedef struct {
    DLINK_NODE     free_list;   /* 独立哨兵：只挂空闲块，size 严格升序 */
    MEM_NODE_INFO *first_node;
} MEM_HEAD_INFO;

typedef struct {
    DLINK_NODE     pool_list;   /* 池链：为多池扩展预留，当前不使用 */
    MEM_HEAD_INFO *head;
} MEM_POOL_INFO;

/*
 * 描述符尺寸：
 *   - 目标机（rv32，指针 4 B）：三份描述符均为 16 B，first_node 落在 pool+32，
 *     用户指针恒 16 B 对齐，这是 ABI 口径；
 *   - 宿主单测（指针 8 B）：dlink 节点随指针变宽，尺寸按实际 sizeof 取值，
 *     使同源算术逻辑（NEXT/PREV/合并）在宿主上等价可测。
 * 两种情况下算法与不变量完全一致，只有指针宽度不同。
 */
#if defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4)
#define MEM_POOL_INFO_SIZE      (16U)
#define MEM_HEAD_INFO_SIZE      (16U)
#define MEM_NODE_INFO_SIZE      (16U)
#else
#define MEM_POOL_INFO_SIZE      ((UINT32)sizeof(MEM_POOL_INFO))
#define MEM_HEAD_INFO_SIZE      ((UINT32)sizeof(MEM_HEAD_INFO))
#define MEM_NODE_INFO_SIZE      ((UINT32)sizeof(MEM_NODE_INFO))
#endif

/* ---- 尺寸与位定义 ---- */
#define MEM_SIZE_MASK           (0x7FFFFFFFU)
#define MEM_USED_MASK           (0x80000000U)
#define MEM_INVALID_SIZE        (0x0U)

#define MEM_PREV_MAGIC_MASK     (0xC0000000U)
#define MEM_PREV_MAGIC          (0x80000000U)
#define MEM_PREV_SIZE_MASK      (0x3FFFFFFFU)

#define MEM_NODE_GET_SIZE(node)         ((node)->size_and_used & MEM_SIZE_MASK)
#define MEM_NODE_SET_SIZE(node, size)   ((node)->size_and_used = (MEM_NODE_GET_USED((node)) | ((size) & MEM_SIZE_MASK)))
#define MEM_NODE_GET_USED(node)         ((node)->size_and_used & MEM_USED_MASK)
#define MEM_NODE_SET_USED(node)         ((node)->size_and_used |= MEM_USED_MASK)
#define MEM_NODE_CLR_USED(node)         ((node)->size_and_used &= (~MEM_USED_MASK))
#define MEM_NODE_GET_DATA(node)         ((UINTPTR)((UINT8 *)(node) + MEM_NODE_INFO_SIZE))

/* prev_size 访问器：对 magic 位透明，业务代码不要直接读该字段 */
#define MEM_NODE_GET_PREV(node)         ((node)->prev_size & MEM_PREV_SIZE_MASK)
#define MEM_NODE_SET_PREV(node, size)   ((node)->prev_size = (MEM_PREV_MAGIC | ((size) & MEM_PREV_SIZE_MASK)))

/* 物理相邻关系：next 由 size 推算，prev 由 prev_size 反向推算 */
#define MEM_NODE_NEXT(node)             ((MEM_NODE_INFO *)((UINT8 *)(node) + MEM_NODE_INFO_SIZE + MEM_NODE_GET_SIZE(node)))
#define MEM_NODE_PREV(node)             ((MEM_NODE_INFO *)((UINT8 *)(node) - MEM_NODE_INFO_SIZE - MEM_NODE_GET_PREV(node)))

/* ---- 错误码（仅在内部函数中使用，对外 API 仍返回指针/NULL） ---- */
#define MEM_BASE_ERR                (0x00020000U)
#define MEM_IS_NULL_ERR             (MEM_BASE_ERR | 0x01U)
#define MEM_SIZE_INVALID_ERR        (MEM_BASE_ERR | 0x02U)
#define MEM_NO_FREE_BLK_ERR         (MEM_BASE_ERR | 0x03U)
#define MEM_NODE_CORRUPTED_ERR      (MEM_BASE_ERR | 0x04U)
#define MEM_DOUBLE_FREE_ERR         (MEM_BASE_ERR | 0x05U)
#define MEM_POOL_NOT_INIT_ERR       (MEM_BASE_ERR | 0x06U)
#define MEM_BOUNDARY_OVERFLOW_ERR   (MEM_BASE_ERR | 0x07U)

/* 布局静态断言：两种配置必须完全一致 */
#if defined(__cplusplus)
#define MEM_STATIC_ASSERT(cond, msg)    static_assert(cond, msg)
#else
#define MEM_STATIC_ASSERT(cond, msg)    _Static_assert(cond, msg)
#endif

#if defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4)
MEM_STATIC_ASSERT(sizeof(MEM_NODE_INFO) == 16U, "block header must be 16 B on rv32");
MEM_STATIC_ASSERT(sizeof(MEM_HEAD_INFO) <= 16U, "head descriptor must fit 16 B");
MEM_STATIC_ASSERT(sizeof(MEM_POOL_INFO) <= 16U, "pool descriptor must fit 16 B");
#endif

UINT32 inner_mem_init(VOID *pool, UINT32 pool_size);
UINT32 inner_mem_alloc(UINT32 size, VOID **out);
UINT32 inner_mem_free(VOID *ptr);

/* 只读调试接口：供宿主机用例与 Shell 观察堆状态，不参与分配逻辑 */
MEM_HEAD_INFO *inner_mem_head(VOID);
UINT8 *inner_mem_heap_start(VOID);
UINT8 *inner_mem_heap_end(VOID);

#endif
