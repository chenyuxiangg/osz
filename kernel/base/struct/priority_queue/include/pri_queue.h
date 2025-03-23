#ifndef __PRI_QUEUE_H__
#define __PRI_QUEUE_H__

#include "dlink.h"

UINT32 pri_queue_init(VOID);
UINT32 pri_queue_enqueue(UINT8 queue_id, DLINK_NODE *node);
DLINK_NODE * pri_queue_dequeue(UINT8 queue_id, DLINK_NODE *node);
DLINK_NODE *pri_queue_top(UINT8 queue_id);
UINT32 pri_queue_size(UINT8 queue_id);

#endif