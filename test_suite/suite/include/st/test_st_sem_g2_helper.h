#ifndef TEST_ST_SEM_G2_HELPER_H
#define TEST_ST_SEM_G2_HELPER_H

#include "comm.h"
#include "sem.h"
#include "task.h"

typedef struct {
    osz_sem_t *sem;
    uint32_t pend_cnt : 8;
    uint32_t post_cnt : 8;
    uint32_t test_end : 1;
    uint32_t wake_task : 3;
    uint32_t reserv : 12;
} st_sem_checker;

typedef struct {
    st_sem_checker *checker;
    uint32_t data;
} st_sem_task_input;

void_t helper_multiple_task_sem_pend_entry(void_t *data);
void_t helper_multiple_task_order_wake_entry(void_t *data);
void_t helper_task_sem_mixed_ops_entry(void_t *data);
void_t helper_task_sem_timeout_wait_entry(void_t *data);

#endif /* TEST_ST_SEM_G2_HELPER_H */
