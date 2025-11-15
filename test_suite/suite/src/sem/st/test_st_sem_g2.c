/**
 * @file test_st_sem_g2.c
 * @brief System test cases group 2 for sem module - Multiple Task Semaphore Synchronization Tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for semaphore multiple task synchronization
 * 
 * Coverage targets:
 * - Function coverage: 100%
 * - Scenario coverage: Multiple task synchronization scenarios
 * - Module interaction: Task scheduling and IPC with multiple tasks
 */

#include "mem.h"
#include "sem.h"
#include "et.h"
#include "test_st_sem_g2_helper.h"

#define TASK_STACK_SIZE_DEFAULT     (0x2000)

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 2: Multiple Task Semaphore Synchronization Tests
// =========================================================================
TEST_GROUP(ET_MODULE_SEM_ST, 2, "Multiple Task Semaphore Synchronization Tests", setup, teardown)
{
    TEST("Test_2_1: Multiple Tasks Waiting on Same Semaphore") {
        // Preconditions: System initialized, multiple tasks created, available semaphore
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Create multiple tasks (e.g., 3 tasks) that wait on the same semaphore
        //   3. Release the semaphore multiple times (e.g., 3 times)
        //   4. Verify each task is woken up in order (if priority based) or all are woken
        //   5. Verify all tasks complete successfully
        // Expected:
        //   - All tasks correctly block on semaphore wait
        //   - Each release wakes one waiting task
        //   - All tasks eventually resume execution and complete
        //   - No tasks remain blocked indefinitely
        
        osz_sem_t *sem = NULL;
        uint32_t ret;
        
        // Step 1: Initialize semaphore with count 0
        ret = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(ret == OS_OK);
        st_sem_checker checker = {
                .sem = sem,
                .pend_cnt = 0,
                .post_cnt = 0,
                .test_end = 0,
        };
        VERIFY(checker.sem != NULL);
        
        uint16_t task1_id, task2_id, task3_id;
        void_t *stack_addr1 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params1 = {
            .name = "task1",
            .stack_base = (uintptr_t)stack_addr1,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_multiple_task_sem_pend_entry,
            .priority = 8,
            .data = &checker
        };
        
        void_t *stack_addr2 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params2 = {
            .name = "task2",
            .stack_base = (uintptr_t)stack_addr2,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_multiple_task_sem_pend_entry,
            .priority = 8,
            .data = &checker
        };

        void_t *stack_addr3 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params3 = {
            .name = "task3",
            .stack_base = (uintptr_t)stack_addr3,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_multiple_task_sem_pend_entry,
            .priority = 8,
            .data = &checker
        };
        // Create 3 tasks with same priority
        ret = osz_create_task(&task1_id, &task_params1);
        VERIFY(ret == OS_OK);
        osz_task_resume(task1_id);
        
        ret = osz_create_task(&task2_id, &task_params2);
        VERIFY(ret == OS_OK);
        osz_task_resume(task2_id);
        
        ret = osz_create_task(&task3_id, &task_params3);
        VERIFY(ret == OS_OK);
        osz_task_resume(task3_id);
        
        // Give tasks time to start and block on semaphore
        osz_msleep(100);
        VERIFY(checker.pend_cnt == 3);
        
        // Step 3: Release semaphore 3 times
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(100);
        VERIFY(checker.post_cnt == 1);
        
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(50);
        VERIFY(checker.post_cnt == 2);
        
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(50);
        VERIFY(checker.post_cnt == 3);

        osz_sem_detach(sem);
    }

    TEST("Test_2_2: Semaphore Wakeup Order with Task Priorities") {
        // Preconditions: System initialized, multiple tasks with different priorities created, check task wake up by FIFO order.
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Create multiple tasks with different priorities that wait on the same semaphore
        //   3. Release the semaphore once
        //   4. Verify the highest priority task is woken up first
        //   5. Release again and verify next highest priority task wakes up
        // Expected:
        //   - Tasks are woken in priority order when semaphore is released
        //   - Higher priority tasks preempt lower priority tasks when woken
        //   - Scheduling behavior conforms to priority rules
        
        osz_sem_t *sem = NULL;
        uint32_t ret;
        
        // Step 1: Initialize semaphore with count 0
        ret = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(ret == OS_OK);
        VERIFY(sem != NULL);
        st_sem_checker checker = {
            .sem = sem,
            .pend_cnt = 0,
            .post_cnt = 0,
            .test_end = 0,
            .wake_task = 0,
        };
        
        uint16_t task1_id, task2_id, task3_id;
        void_t *stack_addr1 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        st_sem_task_input data1 = {
            .checker = &checker,
            .data = 1,
        };
        osz_task_params_t task_params1 = {
            .name = "task_high",
            .stack_base = (uintptr_t)stack_addr1,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_multiple_task_order_wake_entry,
            .priority = 5,  // Highest priority
            .data = &data1,
        };
        
        ret = osz_create_task(&task1_id, &task_params1);
        VERIFY(ret == OS_OK);
        osz_task_resume(task1_id);
        
        void_t *stack_addr2 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        st_sem_task_input data2 = {
            .checker = &checker,
            .data = 2,
        };
        osz_task_params_t task_params2 = {
            .name = "task_med",
            .stack_base = (uintptr_t)stack_addr2,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_multiple_task_order_wake_entry,
            .priority = 10,  // Highest priority
            .data = &data2,
        };
        ret = osz_create_task(&task2_id, &task_params2);
        VERIFY(ret == OS_OK);
        osz_task_resume(task2_id);
        
        void_t *stack_addr3 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        st_sem_task_input data3 = {
            .checker = &checker,
            .data = 4,
        };
        osz_task_params_t task_params3 = {
            .name = "task_low",
            .stack_base = (uintptr_t)stack_addr3,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_multiple_task_order_wake_entry,
            .priority = 15,  // Highest priority
            .data = &data3,
        };
        ret = osz_create_task(&task3_id, &task_params3);
        VERIFY(ret == OS_OK);
        osz_task_resume(task3_id);
        
        // Give tasks time to start and block on semaphore
        osz_msleep(500);
        VERIFY(checker.pend_cnt == 3);
        
        // Step 3: Release semaphore once - should wake highest priority task
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(500);
        
        // Step 4: Verify highest priority task was woken first
        VERIFY(checker.post_cnt == 1);
        VERIFY(checker.sem->field.sem == 0);
        VERIFY(checker.wake_task == 4);
        
        // Step 5: Release again and verify next highest priority task wakes up
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(500);

        VERIFY(checker.post_cnt == 2);
        VERIFY(checker.sem->field.sem == 0);
        VERIFY(checker.wake_task == 6);
        
        // Release third time for lowest priority task
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(500);

        VERIFY(checker.post_cnt == 3);
        VERIFY(checker.sem->field.sem == 0);
        VERIFY(checker.wake_task == 7);

        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);

        VERIFY(checker.post_cnt == 3);
        VERIFY(checker.sem->field.sem == 1);
        
        // Cleanup
        osz_sem_detach(sem);
    }

    TEST("Test_2_3: Semaphore with Mixed Acquire and Release Operations") {
        // Preconditions: System initialized, multiple tasks created, available semaphore
        // Steps:
        //   1. Initialize a semaphore with initial count 2
        //   2. Create tasks that perform both acquire and release operations
        //   3. Verify semaphore count correctly reflects concurrent operations
        //   4. Verify no race conditions or count corruption occur
        // Expected:
        //   - Semaphore count remains consistent despite concurrent operations
        //   - All acquire and release operations return appropriate status
        //   - No tasks experience unexpected blocking or wakeups
        
        osz_sem_t *sem = NULL;
        uint32_t ret;
        
        // Step 1: Initialize semaphore with count 2
        ret = osz_sem_init(NULL, 0, 1, &sem);
        VERIFY(ret == OS_OK);
        VERIFY(sem != NULL);
        st_sem_checker checker = {
            .sem = sem,
            .pend_cnt = 0,
            .post_cnt = 0,
            .test_end = 0,
            .wake_task = 0,
        };
        
        uint16_t task1_id, task2_id;
        void_t *stack_addr1 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params1 = {
            .name = "task_a",
            .stack_base = (uintptr_t)stack_addr1,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_task_sem_mixed_ops_entry,
            .priority = 10,
            .data = &checker
        };
        
        ret = osz_create_task(&task1_id, &task_params1);
        VERIFY(ret == OS_OK);
        osz_task_resume(task1_id);
        
        void_t *stack_addr2 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params2 = {
            .name = "task_a",
            .stack_base = (uintptr_t)stack_addr2,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_task_sem_mixed_ops_entry,
            .priority = 9,
            .data = &checker
        };
        ret = osz_create_task(&task2_id, &task_params2);
        VERIFY(ret == OS_OK);
        osz_task_resume(task2_id);
        
        // Give tasks time to execute operations
        osz_msleep(600);
        
        // Step 3 & 4: Verify operations completed successfully
        VERIFY(checker.post_cnt == 2);  // Both operations should complete
        VERIFY(checker.sem->field.sem == 1);  // Both operations should complete
        
        // Cleanup
        osz_sem_detach(sem);
    }

    TEST("Test_2_4: Semaphore with Timeout and Multiple Waiters") {
        // Preconditions: System initialized, multiple tasks created, available semaphore
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Create multiple tasks that wait on semaphore with different timeouts
        //   3. Let some tasks timeout while others are woken by semaphore release
        //   4. Verify timeout tasks return appropriate error code
        //   5. Verify released tasks return OS_OK
        // Expected:
        //   - Tasks with timeout return TASK_WAIT_TIMEOUT_ERR or timeout error when applicable
        //   - Tasks woken by release return OS_OK
        //   - Semaphore wait list correctly managed during timeouts and releases
        
        osz_sem_t *sem = NULL;
        uint32_t ret;
        
        // Step 1: Initialize semaphore with count 0
        ret = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(ret == OS_OK);
        VERIFY(sem != NULL);
        st_sem_checker checker = {
            .pend_cnt = 0,
            .post_cnt = 0,
            .sem = sem,
            .test_end = 0,
            .wake_task = 0,
        };
        st_sem_task_input data1 = {
            .checker = &checker,
            .data = 1,
        };
        
        uint16_t task1_id, task2_id, task3_id;
        void_t *stack_addr1 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params = {
            .name = "task_short",
            .stack_base = (uintptr_t)stack_addr1,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_task_sem_timeout_wait_entry,
            .priority = 10,
            .data = &data1
        };
        
        ret = osz_create_task(&task1_id, &task_params);
        VERIFY(ret == OS_OK);
        osz_task_resume(task1_id);
        
        st_sem_task_input data2 = {
            .checker = &checker,
            .data = 2,
        };
        void_t *stack_addr2 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params2 = {
            .name = "task_med",
            .stack_base = (uintptr_t)stack_addr2,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_task_sem_timeout_wait_entry,
            .priority = 10,
            .data = &data2
        };
        ret = osz_create_task(&task2_id, &task_params2);
        VERIFY(ret == OS_OK);
        osz_task_resume(task2_id);
        
        st_sem_task_input data3 = {
            .checker = &checker,
            .data = 3,
        };
        void_t *stack_addr3 = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t task_params3 = {
            .name = "task_short",
            .stack_base = (uintptr_t)stack_addr3,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .thread = helper_task_sem_timeout_wait_entry,
            .priority = 10,
            .data = &data3
        };
        ret = osz_create_task(&task3_id, &task_params3);
        VERIFY(ret == OS_OK);
        osz_task_resume(task3_id);
        
        // Give tasks time to start and block on semaphore
        osz_msleep(500);

        VERIFY(checker.pend_cnt == 3);
        
        // Release semaphore twice to wake the other two tasks
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(50);
        VERIFY(checker.post_cnt == 1);
        
        ret = osz_sem_post(sem);
        VERIFY(ret == OS_OK);
        osz_msleep(50);
        VERIFY(checker.post_cnt == 2);

        osz_msleep(600);
        VERIFY(checker.post_cnt == 3);
        VERIFY(checker.test_end == 1);
        
        // Cleanup
        osz_sem_detach(sem);
    }
}
