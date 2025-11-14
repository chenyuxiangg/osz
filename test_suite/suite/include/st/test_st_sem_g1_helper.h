#ifndef TEST_ST_SEM_G1_HELPER_H
#define TEST_ST_SEM_G1_HELPER_H

#include "comm.h"
#include "sem.h"
#include "task.h"
#include "inner_task_err.h"

#define PRIORITY_FOR_TEST_SEM_DEFAULT   (0x9)

typedef struct {
    osz_sem_t *sem;
    uint32_t pend_cnt : 8;
    uint32_t post_cnt : 8;
    uint32_t test_end : 1;
    uint32_t reserve: 15;
} st_sem_checker;

void_t helper_task_sem_pend_entry(void_t *data);
void_t helper_task_sem_post_entry(void_t *data);
void_t helper_task_sem_multiple_pend_entry(void_t *data);
void_t helper_task_sem_multiple_post_entry(void_t *data);

#endif // TEST_ST_SEM_G1_HELPER_H
