/**
 * @file test_st_sem_g4_helper.h
 * @brief Helper functions for semaphore timeout and scheduling interaction tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 */

#ifndef TEST_ST_SEM_G4_HELPER_H
#define TEST_ST_SEM_G4_HELPER_H

#include "ztype.h"
#include "sem.h"
#include "task.h"
#include "mem.h"

#define PRIORITY_FOR_TEST_SEM_DEFAULT   (0x9)

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

uint32_t helper_task_create(uint8_t *task_name, uint32_t prio, uint32_t stack_size, task_callback_t func, void_t *data);
void_t helper_4_1_task_entry(void_t *data);

#endif /* TEST_ST_SEM_G4_HELPER_H */
