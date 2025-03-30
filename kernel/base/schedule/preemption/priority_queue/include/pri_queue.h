#ifndef __PRI_QUEUE_H__
#define __PRI_QUEUE_H__

#include "comm.h"
#include "dlink.h"

typedef enum {
    EQ_MODE_HEAD = 0,
    EQ_MODE_TAIL
} EQ_MODE;

UINT32 pri_queue_enqueue(UINT8 queue_id, DLINK_NODE *node, EQ_MODE mode);
DLINK_NODE * pri_queue_dequeue(UINT8 queue_id, DLINK_NODE *node);
DLINK_NODE *pri_queue_top(UINT8 queue_id);
UINT32 pri_queue_size(UINT8 queue_id);
UINT32 pri_queue_get_bitmap_low_bit(VOID);

#endif