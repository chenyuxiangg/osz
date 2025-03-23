#ifndef __DLINK_H__
#define __DLINK_H__
#include "ztype.h"
#include "offset.h"

typedef struct DLINK_NODE {
    struct DLINK_NODE *pre;  // 指向前一个节点
    struct DLINK_NODE *next; // 指向下一个节点
} DLINK_NODE;

#define DLINK_GET_NEXT(node)                ((node)->next)
#define DLINK_GET_PRE(node)                 ((node)->pre)
#define DLINK_EMPTY(node)                   (((node)->next == (node)->pre) ? 1 : 0)
#define DLINK_ENTRY(type, member, item)     ((type *)(VOID *)((char *)(item) - OFFSET(type, member)))
#define DLINK_FOREACH(pos, node)            \
    for (pos = (node)->next; pos != NULL && pos != (node); pos = pos->next)

VOID dlink_init(DLINK_NODE *head);
VOID dlink_insert_head(DLINK_NODE *pos, DLINK_NODE *node);
VOID dlink_insert_tail(DLINK_NODE *pos, DLINK_NODE *node);
DLINK_NODE *dlink_del_node(DLINK_NODE *node);

#endif