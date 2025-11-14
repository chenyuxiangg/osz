#include "test_st_sem_g2_helper.h"
#include "inner_task_err.h"

void_t helper_multiple_task_sem_pend_entry(void_t *data)
{
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->pend_cnt += 1;
    ARCH_INT_UNLOCK(intsave);
    uint32_t ret = osz_sem_pend(checker->sem, WAIT_FOREVER);
    if (ret == TASK_WAIT_WAKE_UP_ERR) {
        ARCH_INT_LOCK(intsave);
        checker->post_cnt += 1;
        ARCH_INT_UNLOCK(intsave);
    }
}

void_t helper_multiple_task_order_wake_entry(void_t *data)
{
    st_sem_task_input *input = (st_sem_task_input *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    input->checker->pend_cnt++;
    ARCH_INT_UNLOCK(intsave);

    osz_sem_pend(input->checker->sem, WAIT_FOREVER);

    ARCH_INT_LOCK(intsave);
    input->checker->wake_task |= input->data;
    input->checker->post_cnt++;
    ARCH_INT_UNLOCK(intsave);
}

void_t helper_task_sem_mixed_ops_entry(void_t *data)
{
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->pend_cnt++;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_pend(checker->sem, WAIT_FOREVER);

    osz_msleep(200);
    ARCH_INT_LOCK(intsave);
    checker->post_cnt++;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_post(checker->sem);
}

void_t helper_task_sem_timeout_wait_entry(void_t *data)
{
    st_sem_task_input *input = (st_sem_task_input *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    input->checker->pend_cnt++;
    ARCH_INT_UNLOCK(intsave);

    uint32_t timeout = 0;
    if (input->data == 3) {
        timeout = WAIT_FOREVER;
    } else if (input->data == 2) {
        timeout = WAIT_FOREVER;
    } else if (input->data == 1) {
        timeout = 1000;
    }
    osz_sem_pend(input->checker->sem, timeout);

    ARCH_INT_LOCK(intsave);
    input->checker->post_cnt++;
    if (input->data == 3) {
        input->checker->test_end = 1;
    }
    ARCH_INT_UNLOCK(intsave);
}