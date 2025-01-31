#ifndef __PLIC_H__
#define __PLIC_H__
#include "comm.h"
#include "plic_errno.h"

/*
* These macro are inner, should not to use directly.
*/
#define PLIC_PRIORITY_BASE_OFFSET               (0x0)
#define PLIC_PRIORITY(source_id)                (PLIC_PRIORITY_BASE_OFFSET + 0x4 * (source_id))
#define PLIC_PENDING_BASE_OFFSET                (0x1000)
#define PLIC_ENABLE_BASE_OFFSET                 (0x2000)
#define PLIC_PRIORITY_TH_BASE_OFFSET            (0x200000)
#define PLIC_PRIORITY_TH(context_id)            (PLIC_PRIORITY_TH_BASE_OFFSET + 0x1000 * (context_id))
#define PLIC_CLAIM_BASE_OFFSET                  (0x200004)
#define PLIC_CLAIM(context_id)                  (PLIC_CLAIM_BASE_OFFSET + 0x1000 * (context_id))
#define PLIC_COMPLETE_BASE_OFFSET               (0x200004)
#define PLIC_COMPLETE(context_id)               (PLIC_COMPLETE_BASE_OFFSET + 0x1000 * (context_id))

/*
* Usually, we should use these macro to operate PLIC.
*/
#define PLIC_SET_IE(base, context_id, source_id) \
do { \
    *(volatile UINT32 *)((base) + PLIC_ENABLE_BASE_OFFSET + (0x80 * (context_id)) + ((source_id)/32 * 4)) |= (1 << ((source_id) & 0x1f)); \
} while(0);

#define PLIC_GET_IP(base, source_id, ret) \
do { \
    (ret) = *(volatile UINT32 *)((base) + PLIC_PENDING_BASE_OFFSET + (source_id)/32 * 4); \
} while(0);

#define PLIC_GET_IP_AND_CLEAN(base, context_id, ret) \
do { \
    (ret) = *(volatile UINT32 *)((base) + PLIC_CLAIM((context_id))); \
} while(0);

#define PLIC_NOTIFY_COMPLETE(base, context_id, source_id) \
do { \
    *(volatile UINT32 *)((base) + PLIC_COMPLETE((context_id))) = (source_id); \
} while(0);

#define PLIC_SET_PRIORITY(base, source_id, pri) \
do { \
    *(volatile UINT32 *)((base) + PLIC_PRIORITY((source_id))) = (pri); \
} while(0);

#define PLIC_SET_PRIORITY_TH(base, context_id, pri_th) \
do { \
    *(volatile UINT32 *)((base) + PLIC_PRIORITY_TH((context_id))) = (pri_th); \
} while(0);

#endif