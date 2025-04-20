#ifndef __EVENT_H__
#define __EVENT_H__
#include "comm.h"

#define EVENT_WAIT_FOREVER      (0xFFFFFFFF)

typedef enum {
    EVENT_OP_OR = 1,
    EVENT_OP_AND = 2,
    EVENT_OP_CLR = 4,
} EVENT_OP;

typedef struct {
    UINT32 event_wait;
    EVENT_OP event_op;
    UINT32 event_occur;
} EVENT_CB;

UINT32 event_read(EVENT_CB *ecb, UINT32 timeout);
VOID event_write(UINT32 events);
UINT32 event_poll(EVENT_CB *ecb);

#endif