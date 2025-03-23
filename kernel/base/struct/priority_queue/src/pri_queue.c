#include "qualifer.h"
#include "assert.h"
#include "inner_pri_queue_err.h"
#include "pri_queue.h"

#define PRI_QUEUE_MAX_NUM       OSZ_CFG_PRI_QUEUE_LIMIT

STATIC DLINK_NODE g_pri_queues[PRI_QUEUE_MAX_NUM];
STATIC UINT32 g_pri_bitmap;

UINT32 pri_queue_init(VOID)
{
    for (UINT32 i = 0; i < PRI_QUEUE_MAX_NUM; ++i) {
        dlink_init(&(g_pri_queues[i]));
    }
    g_pri_bitmap = 0;
    return OS_OK;
}

UINT32 pri_queue_enqueue(UINT8 queue_id, DLINK_NODE *node)
{
    ASSERT(node);
    if (node->next != NULL) {
        return PRI_QUEUE_ENQUEUE_IN_QUEUE_ERR;
    }
    if (DLINK_EMPTY(&(g_pri_queues[queue_id]))) {
        g_pri_bitmap |= (1 << queue_id);
    }
    dlink_insert_tail(&(g_pri_queues[queue_id]), node);
    return OS_OK;
}

DLINK_NODE * pri_queue_dequeue(UINT8 queue_id, DLINK_NODE *node)
{
    ASSERT(node);
    if (DLINK_EMPTY(&(g_pri_queues[queue_id]))) {
        return NULL;
    }
    dlink_del_node(node);
    if (DLINK_EMPTY(&(g_pri_queues[queue_id]))) {
        g_pri_bitmap &= ~(1 << queue_id);
    }
    return node;
}

DLINK_NODE *pri_queue_top(UINT8 queue_id)
{
    if (DLINK_EMPTY(&(g_pri_queues[queue_id]))) {
        return NULL;
    }
    return (g_pri_queues[queue_id].next);
}

UINT32 pri_queue_size(UINT8 queue_id)
{
    UINT32 queue_size = 0;
    if (DLINK_EMPTY(&(g_pri_queues[queue_id]))) {
        return queue_size;
    }
    DLINK_NODE *it = NULL;
    DLINK_FOREACH(it, &(g_pri_queues[queue_id])) {
        queue_size++;
    }
    return queue_size;
}