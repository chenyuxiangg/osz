#ifndef __SORTLINK_H__
#define __SORTLINK_H__
#include "comm.h"
#include "dlink.h"

typedef struct {
    DLINK_NODE  list;
    UINT32      timeout;
} SORT_LINK;
typedef SORT_LINK osz_sortlink_t;

#define SORTLINK_LIST(sortlink)        ((sortlink).list)
#define PSORTLINK_LIST(psortlink)      ((psortlink)->list)
#define SORTLINK_TIMEOUT(sortlink)      ((sortlink).timeout)
#define PSORTLINK_TIMEOUT(psortlink)    ((psortlink)->timeout)

UINT32 sortlink_init(SORT_LINK *head);
UINT32 sortlink_insert(SORT_LINK *head, SORT_LINK *link);
UINT32 sortlink_delete(SORT_LINK *link);

#endif