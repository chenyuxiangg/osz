#include "dlink.h"
#include "uart.h"

typedef struct {
    DLINK_NODE list;
    int x;
} TS;

void test_dlink(void)
{
    DLINK_NODE head;
    dlink_init(&head);
    if (DLINK_EMPTY(&head)) {
        drv_uart_write("[dlink] test init success!\n");
    }

    DLINK_NODE node1;
    DLINK_NODE node2;
    dlink_insert_head(&head, &node1);
    dlink_insert_tail(&head, &node2);
    if (DLINK_GET_NEXT(&head) == DLINK_GET_PRE(&node1)) {
        drv_uart_write("[dlink] test insert success!\n");
    }

    DLINK_NODE *pos = NULL;
    DLINK_FOREACH(pos, &head) {
        if (DLINK_GET_NEXT(pos) == &node2) {
            drv_uart_write("[dlink] test for 0 success!\n");
            continue;
        }
        if (DLINK_GET_NEXT(pos) == &node1) {
            drv_uart_write("[dlink] test for 1 success!\n");
            continue;
        }
        if (DLINK_GET_NEXT(pos) == &head) {
            drv_uart_write("[dlink] test for 2 success!\n");
            continue;
        }
    }

    TS t1 = {
        .x = 1
    };

    /* 
    * === an error example for STRUCT_ENTRY ===
    * TS t1 = {
    *    .list = node1,
    *    .x = 1
    * };
    * witch is error for using STRUCT_ENTRY(TS, list, &node1) to get t1 addr
    * because node1 is not TS member.
    */


    TS *pt = STRUCT_ENTRY(TS, list, &(t1.list));
    if (pt->x == 1) {
        drv_uart_write("[dlink] test get entry success!\n");
    }

    DLINK_NODE *tmp = dlink_del_node(&node1);
    (void)tmp;
    if (DLINK_GET_PRE(&head) == &node2 && DLINK_GET_NEXT(&head) == &node2) {
        drv_uart_write("[dlink] test del success!\n");
    }
}