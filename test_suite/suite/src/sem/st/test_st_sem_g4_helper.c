/**
 * @file test_st_sem_g4_helper.c
 * @brief Helper functions implementation for semaphore timeout and scheduling interaction tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 */

#include "test_st_sem_g4_helper.h"
#include "inner_task_err.h"
#include "sem.h"
#include "task.h"
#include "et.h"

void_t helper_4_1_task_entry(void_t *data)
{
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->pend_cnt++;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_pend(checker->sem, 0x500);
    ARCH_INT_LOCK(intsave);
    checker->post_cnt++;
    checker->test_end = 1;
    ARCH_INT_UNLOCK(intsave);
}