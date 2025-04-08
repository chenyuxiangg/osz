#include "sortlink.h"
#include "inner_sortlink_err.h"

#define SORTLINK_MAX_TIMEOUT        (0xffffffff)
#define SORTLINK_INVALID_TIMEOUT    (0)

SORT_LINK SECTION_KERNEL_INIT_DATA g_task_sortlink;

UINT32 sortlink_init(SORT_LINK *head)
{
    ASSERT(head);
    dlink_init(&(PSORTLINK_LIST(head)));
    PSORTLINK_TIMEOUT(head) = SORTLINK_MAX_TIMEOUT;
    return OS_OK;
}

UINT32 sortlink_insert(SORT_LINK *link)
{
    ASSERT(link);
    DLINK_NODE *iter = NULL;
    if (PSORTLINK_TIMEOUT(link) == SORTLINK_INVALID_TIMEOUT) {
        return SORTLINK_TIMEOUT_INVALID;
    }
    if (DLINK_EMPTY(&(SORTLINK_LIST(g_task_sortlink)))) {
        dlink_insert_tail(&(SORTLINK_LIST(g_task_sortlink)), &(PSORTLINK_LIST(link)));
        return OS_OK;
    }
    DLINK_FOREACH(iter, &(SORTLINK_LIST(g_task_sortlink))) {
        SORT_LINK * sl = STRUCT_ENTRY(SORT_LINK, list, iter);
        if (PSORTLINK_TIMEOUT(sl) <= PSORTLINK_TIMEOUT(link)) {
            PSORTLINK_TIMEOUT(link) -= PSORTLINK_TIMEOUT(sl);
        } else {
            PSORTLINK_TIMEOUT(sl) -= PSORTLINK_TIMEOUT(link);
            dlink_insert_head(&(PSORTLINK_LIST(sl)), &(PSORTLINK_LIST(link)));
            break;
        }
    }
    if (iter == &SORTLINK_LIST(g_task_sortlink)) {
        dlink_insert_head(&(SORTLINK_LIST(g_task_sortlink)), &(PSORTLINK_LIST(link)));
    }
    return OS_OK;
}

UINT32 sortlink_delete(SORT_LINK *link)
{
    ASSERT(link);
    dlink_del_node(&(PSORTLINK_LIST(link)));
    return OS_OK;
}