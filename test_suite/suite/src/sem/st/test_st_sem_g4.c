/**
 * @file test_st_sem_g4.c
 * @brief System test cases group 4 for sem module - Semaphore Timeout and Scheduling Interaction Tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for semaphore timeout and scheduling interaction
 * 
 * Coverage targets:
 * - Function coverage: 100%
 * - Scenario coverage: Timeout and scheduling scenarios
 * - Module interaction: Task scheduling, timeout handling, and IPC
 */

#include "mem.h"
#include "sem.h"
#include "et.h"
#include "inner_task_err.h"
#include "test_st_sem_g4_helper.h"

#define TASK_STACK_SIZE_DEFAULT     (0x2000)

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 4: Semaphore Timeout and Scheduling Interaction Tests
// =========================================================================
TEST_GROUP(ET_MODULE_SEM_ST, 4, "Semaphore Timeout and Scheduling Interaction Tests", setup, teardown)
{
    TEST("Test_4_1: Timeout and Release Race Condition") {
        // Preconditions: System initialized, task created, available semaphore
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Create a task that waits on semaphore with a short timeout
        //   3. Simultaneously release the semaphore from another context
        //   4. Verify correct behavior (either timeout or success, but not both)
        // Expected:
        //   - Either timeout or success is returned, not both
        //   - No double wakeup or missed wakeup occurs
        //   - Semaphore state remains consistent
        //   - System handles race condition correctly
        
        osz_sem_t *sem = NULL;
        uint32_t result;
        
        // Step 1: Initialize semaphore with initial count 0
        result = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(result == OS_OK);
        VERIFY(sem != NULL);
        st_sem_checker checker = {
            .pend_cnt = 0,
            .post_cnt = 0,
            .sem = sem,
            .test_end = 0,
            .wake_task = 0,
        };

        uint32_t res = helper_task_create("test_4_1_task1", PRIORITY_FOR_TEST_SEM_DEFAULT, TASK_STACK_SIZE_DEFAULT, (task_callback_t)helper_4_1_task_entry, (void_t *)&checker);
        VERIFY(res == OS_OK);

        osz_msleep(0x100);
        VERIFY(checker.pend_cnt == 1);
        VERIFY(checker.sem->field.sem == 0);
        VERIFY(checker.post_cnt == 0);

        osz_sem_post(checker.sem);
        osz_msleep(0x100);

        VERIFY((checker.test_end == 1) && (checker.post_cnt == 1));

        osz_sem_detach(sem);
    }
}
