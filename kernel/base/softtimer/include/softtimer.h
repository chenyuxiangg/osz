#ifndef __SOFTTIMER_H__
#define __SOFTTIMER_H__
#include "comm.h"
#include "sortlink.h"

typedef VOID (*softtimer_callback)(VOID *args);

typedef enum {
    SOFTTIMER_FLAG_REPEAT = 0x01,
    SOFTTIMER_FLAG_ONESHOT = 0x02,
    SOFTTIMER_FLAG_AUTOSTART = 0x04,
    SOFTTIMER_FLAG_USED = 0x08,
} SOFTTIMER_ATTR;

typedef enum {
    SOFTTIMER_CMD_SET_TIME = 0x01,
    SOFTTIMER_CMD_GET_TIME = 0x02,
    SOFTTIMER_CMD_SET_ONESHOT = 0x03,
    SOFTTIMER_CMD_SET_REPEAT = 0x04
} SOFTTIMER_CMD;

typedef struct {
    SORT_LINK link;

    CHAR *name;
    softtimer_callback func;
    VOID *args;
    UINT32 flags : 8;
    UINT32 load : 24;
} SOFTTIMER_CB;

typedef struct {
    softtimer_callback func;
    CHAR *name;
    VOID *args;
    UINT32 timeout;
    UINT8 flags;
} SOFTTIMER_PARAMS;

UINT32 softtimer_create(UINT32 *swt_id, SOFTTIMER_PARAMS *params);
UINT32 softtimer_delete(UINT32 swt_id);
VOID softtimer_start(UINT32 swt_id);
VOID softtimer_stop(UINT32 swt_id);
VOID softtimer_ctrl(UINT32 swt_id, SOFTTIMER_CMD cmd, VOID *arg);
VOID os_update_swtmr(VOID);
#endif