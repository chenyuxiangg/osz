#include "test_st_sem_g1_helper.h"

void_t helper_task_sem_pend_entry(void_t *data)
{
    if (data == NULL) {
        printf("%s, %d\n", __FUNCTION__, __LINE__);
        return;
    }
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->pend_cnt = 1;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_pend(checker->sem, WAIT_FOREVER);
    ARCH_INT_LOCK(intsave);
    checker->test_end = 1;
    ARCH_INT_UNLOCK(intsave);
}

void_t helper_task_sem_post_entry(void_t *data)
{
    if (data == NULL) {
        printf("%s, %d\n", __FUNCTION__, __LINE__);
        return;
    }
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->post_cnt = 1;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_post(checker->sem);
}

void_t helper_task_sem_multiple_pend_entry(void_t *data)
{
    if (data == NULL) {
        printf("%s, %d\n", __FUNCTION__, __LINE__);
        return;
    }
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    
    // Perform 3 pend operations
    for (int i = 0; i < 3; i++) {
        osz_sem_pend(checker->sem, WAIT_FOREVER);
        ARCH_INT_LOCK(intsave);
        checker->pend_cnt++;
        ARCH_INT_UNLOCK(intsave);
    }
}

void_t helper_task_sem_multiple_post_entry(void_t *data)
{
    if (data == NULL) {
        printf("%s, %d\n", __FUNCTION__, __LINE__);
        return;
    }
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    
    // Perform 3 post operations
    for (int i = 0; i < 3; i++) {
        osz_sem_post(checker->sem);
        ARCH_INT_LOCK(intsave);
        checker->post_cnt++;
        ARCH_INT_UNLOCK(intsave);
    }
    
    // Wait a bit for the pend task to complete
    osz_msleep(100);
    ARCH_INT_LOCK(intsave);
    checker->test_end = 1;
    ARCH_INT_UNLOCK(intsave);
}