#include "ztype.h"
#include "assert.h"
#include "dlink.h"

VOID dlink_init(DLINK_NODE *head)
{
    ASSERT(head);

    head->next = head;
    head->pre = head;
}

VOID dlink_insert_head(DLINK_NODE *pos, DLINK_NODE *node)
{
    ASSERT(node);
    ASSERT(pos);

    node->next = pos;
    node->pre = pos->pre;
    pos->pre->next = node;
    pos->pre = node;
}

VOID dlink_insert_tail(DLINK_NODE *pos, DLINK_NODE *node)
{
    ASSERT(node);
    ASSERT(pos);

    pos->next->pre = node;      // node1->pre = node2
    node->next = pos->next;     // node2->next = node1
    node->pre = pos;            // node2->pre = head
    pos->next = node;           // head->next = node2
}

DLINK_NODE *dlink_del_node(DLINK_NODE *node)
{
    ASSERT(node);
    
    node->pre->next = node->next;
    node->next->pre = node->pre;
    node->next = node->pre = NULL;
    return node;
}