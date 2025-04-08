#include "comm.h"
#include "pri_queue.h"
#include "inner_pri_queue_err.h"

void test_pri_queue(void)
{
    // pri_queue_init();
    bool flag = false;
    UINT32 ret = OS_OK;
    for (int i = 0; i < 32; ++i) {
        if (pri_queue_size(i) != 0) {
            flag = true;
        }
    }
    if (flag) {
        printf("[pri_queue]error: pri_queue_init or pri_queue_size err.\n");
    }
    flag = true;
    DLINK_NODE l1, l2, l3;
    l1.next = l1.pre = NULL;
    l2.next = l2.pre = NULL;
    l3.next = l3.pre = NULL;
    ret = pri_queue_enqueue(1, &l1, EQ_MODE_HEAD);
    if (ret != OS_OK) {
        flag = false;
        printf("[pri_queue]error: pri_queue_enqueue err. ret: %#x\n", ret);
    }
    ret = pri_queue_enqueue(2, &l1, EQ_MODE_HEAD);
    if (ret != PRI_QUEUE_ENQUEUE_IN_QUEUE_ERR) {
        flag = false;
        printf("[pri_queue]error: same node repet enqueue. ret: %#x\n", ret);
    }
    ret = OS_OK;
    pri_queue_enqueue(1, &l2, EQ_MODE_HEAD);
    pri_queue_enqueue(1, &l3, EQ_MODE_HEAD);
    if (pri_queue_top(1) != &l1) {
        flag = false;
        printf("[pri_queue]error: get queue top err.\n");
    }
    DLINK_NODE *tmp = pri_queue_dequeue(1, pri_queue_top(1));
    if (tmp != &l1 || pri_queue_top(1) != &l2) {
        flag = false;
        printf("[pri_queue]error: dequeue err.\n");
    }
    if ((tmp->next != tmp->pre) && (tmp->next != NULL)) {
        flag = false;
        printf("[pri_queue]error: dequeue err.\n");
    }
    if (pri_queue_size(1) != 2) {
        flag = false;
        printf("[pri_queue]error: get size err.\n");
    }
    if (flag) {
        printf("[pri_queue]test successful.\n");
    }
}