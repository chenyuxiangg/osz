#include "inner_pri_queue_err.h"
#include "pri_queue.h"

#define PRI_QUEUE_MAX_NUM           OSZ_CFG_PRI_QUEUE_LIMIT
#define PRI_QUEUE_BITMAP_MAX_BIT    (0x20)

STATIC DLINK_NODE SECTION_KERNEL_INIT_DATA g_task_pri_queues[PRI_QUEUE_MAX_NUM];
STATIC UINT32 SECTION_KERNEL_INIT_DATA g_pri_bitmap;

STATIC SECTION_KERNEL_INIT_TEXT UINT32 inner_pri_queue_init(VOID)
{
    for (UINT32 i = 0; i < PRI_QUEUE_MAX_NUM; ++i) {
        dlink_init(&(g_task_pri_queues[i]));
    }
    g_pri_bitmap = 0;
    return OS_OK;
}
MODULE_INIT(inner_pri_queue_init, l1)

UINT32 pri_queue_enqueue(UINT8 queue_id, DLINK_NODE *node, EQ_MODE mode)
{
    ASSERT(node);
    if (node->next != NULL) {
        return PRI_QUEUE_ENQUEUE_IN_QUEUE_ERR;
    }
    if (DLINK_EMPTY(&(g_task_pri_queues[queue_id]))) {
        g_pri_bitmap |= (1 << queue_id);
    }
    if (mode == EQ_MODE_HEAD) {
        dlink_insert_head(&(g_task_pri_queues[queue_id]), node);
    } else if (mode == EQ_MODE_TAIL) {
        dlink_insert_tail(&(g_task_pri_queues[queue_id]), node);
    }
    return OS_OK;
}

DLINK_NODE * pri_queue_dequeue(UINT8 queue_id, DLINK_NODE *node)
{
    ASSERT(node);
    if (DLINK_EMPTY(&(g_task_pri_queues[queue_id]))) {
        return NULL;
    }
    dlink_del_node(node);
    if (DLINK_EMPTY(&(g_task_pri_queues[queue_id]))) {
        g_pri_bitmap &= ~(1 << queue_id);
    }
    return node;
}

DLINK_NODE *pri_queue_top(UINT8 queue_id)
{
    if (queue_id < 0 || queue_id >= PRI_QUEUE_MAX_NUM) {
        printf("[%s, %d] queue id is error!(id: %d)\n", __FUNCTION__, __LINE__, queue_id);
        return NULL;
    }
    if (DLINK_EMPTY(&(g_task_pri_queues[queue_id]))) {
        printf("[%s, %d] priority is empty!\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    return (g_task_pri_queues[queue_id].next);
}

UINT32 pri_queue_size(UINT8 queue_id)
{
    UINT32 queue_size = 0;
    if (DLINK_EMPTY(&(g_task_pri_queues[queue_id]))) {
        return queue_size;
    }
    DLINK_NODE *it = NULL;
    DLINK_FOREACH(it, &(g_task_pri_queues[queue_id])) {
        queue_size++;
    }
    return queue_size;
}

UINT32 pri_queue_get_bitmap_low_bit(VOID)
{
    for (int i = 0; i < OSZ_CFG_PRI_QUEUE_LIMIT; ++i) {
        if (((g_pri_bitmap >> i) & 1) == 1) {
            return i;
        }
    }
    return OS_NOK;
}