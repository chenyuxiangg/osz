/**
 * @file test_st_sem_g1.c
 * @brief System test cases group 1 for sem module - Basic Synchronization Function Tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for semaphore basic synchronization functionality
 * 
 * Coverage targets:
 * - Function coverage: 100%
 * - Scenario coverage: Basic synchronization scenarios
 * - Module interaction: Task scheduling and basic IPC
 */

#include "sem.h"
#include "mem.h"
#include "et.h"
#include "test_st_sem_g1_helper.h"

#define TASK_STACK_SIZE_DEFAULT     (0x2000)

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 1: Semaphore Basic Synchronization Function Tests
// =========================================================================
TEST_GROUP(ET_MODULE_SEM_ST, 1, "Semaphore Basic Synchronization Function Tests", setup, teardown)
{
    TEST("Test_1_1: Single Task Semaphore Acquire and Release") {
        // Preconditions: System initialized, available static semaphore object
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. TaskA Acquire the semaphore (should pending immediately)
        //   3. TaskB Release the semaphore (should wake up TaskA)
        //   4. Verify semaphore count returns to initial value
        // Expected:
        //   - Semaphore acquire returns OS_OK
        //   - Semaphore release returns OS_OK
        //   - Semaphore count correctly maintained

        osz_sem_t *sem = NULL;
        uint32_t res = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(res == OS_OK);

        st_sem_checker checker = {
            .sem = sem,
            .pend_cnt = 0,
            .post_cnt = 0,
            .test_end = 0,
        };

        uint16_t task_pend_id = 0;
        void_t *pend_stack_addr = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t params = {
            .name = "Test_Sem_Pend",
            .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
            .stack_base = (uintptr_t)pend_stack_addr,
            .stack_attr = STACK_MEM_DYNAMIC,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .thread = (task_callback_t)helper_task_sem_pend_entry,
            .data = (void_t *)(&checker),
        };
        res = osz_create_task(&task_pend_id, &params);
        VERIFY(res == OS_OK);
        osz_task_resume(task_pend_id);

        while (checker.pend_cnt == 0) {
            osz_msleep(30);
        }
        
        VERIFY(checker.pend_cnt == 1);
        VERIFY(checker.post_cnt == 0);
        VERIFY(checker.sem->field.sem == 0);

        uint16_t task_post_id = 0;
        void_t *post_stack_addr = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t params1 = {
            .name = "Test_Sem_Post",
            .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .stack_base = (uintptr_t)post_stack_addr,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .thread = (task_callback_t)helper_task_sem_post_entry,
            .data = (void_t *)(&checker),
        };
        res = osz_create_task(&task_post_id, &params1);
        VERIFY(res == OS_OK);
        osz_task_resume(task_post_id);

        while (checker.post_cnt == 0) {
            osz_msleep(30);
        }

        VERIFY(checker.pend_cnt == 1);
        VERIFY(checker.post_cnt == 1);

        while (checker.test_end == 0) {
            osz_msleep(30);
        }
        VERIFY(checker.sem->field.sem == 0);

        // clean
        res = osz_sem_detach(sem);
        VERIFY(res == OS_OK);
    }

    TEST("Test_1_2: Semaphore Counting Function Verification") {
        // Preconditions: System initialized, available static semaphore object
        // Steps:
        //   1. Initialize a semaphore with initial count 3
        //   2. Perform multiple acquire operations (3 times)
        //   3. Verify each acquire succeeds and count decreases
        //   4. Perform release operations to restore count
        //   5. Verify semaphore count returns to initial value
        // Expected:
        //   - All acquire operations return OS_OK
        //   - Semaphore count correctly decreases with each acquire
        //   - Release operations correctly increase count
        //   - Final count matches initial value
        
        osz_sem_t *sem = NULL;
        uint32_t res = osz_sem_init(NULL, 0, 3, &sem);
        VERIFY(res == OS_OK);

        st_sem_checker checker = {
            .sem = sem,
            .pend_cnt = 0,
            .post_cnt = 0,
            .test_end = 0,
        };

        // Create task to perform 3 pend operations
        uint16_t task_pend_id = 0;
        void_t *pend_stack_addr = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t params = {
            .name = "Test_Sem_Pend_3",
            .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
            .stack_base = (uintptr_t)pend_stack_addr,
            .stack_attr = STACK_MEM_DYNAMIC,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .thread = (task_callback_t)helper_task_sem_multiple_pend_entry,
            .data = (void_t *)(&checker),
        };
        res = osz_create_task(&task_pend_id, &params);
        VERIFY(res == OS_OK);
        osz_task_resume(task_pend_id);

        // Wait for all 3 pend operations to complete
        while (checker.pend_cnt < 3) {
            osz_msleep(30);
        }
        
        VERIFY(checker.pend_cnt == 3);
        VERIFY(checker.post_cnt == 0);
        VERIFY(checker.sem->field.sem == 0);

        // Create task to perform 3 post operations
        uint16_t task_post_id = 0;
        void_t *post_stack_addr = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t params1 = {
            .name = "Test_Sem_Post_3",
            .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .stack_base = (uintptr_t)post_stack_addr,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .thread = (task_callback_t)helper_task_sem_multiple_post_entry,
            .data = (void_t *)(&checker),
        };
        res = osz_create_task(&task_post_id, &params1);
        VERIFY(res == OS_OK);
        osz_task_resume(task_post_id);

        // Wait for all 3 post operations to complete
        while (checker.post_cnt < 3) {
            osz_msleep(30);
        }

        VERIFY(checker.pend_cnt == 3);
        VERIFY(checker.post_cnt == 3);

        // Wait for test to complete
        while (checker.test_end == 0) {
            osz_msleep(30);
        }
        VERIFY(checker.sem->field.sem == 3);

        // Clean up
        res = osz_sem_detach(sem);
        VERIFY(res == OS_OK);
    }

    TEST("Test_1_3: Basic Task Synchronization Scenario") {
        // Preconditions: System initialized, two tasks created, available semaphore
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Task A waits on semaphore (should block)
        //   3. Task B releases semaphore (should wake Task A)
        //   4. Verify Task A resumes execution
        //   5. Verify both tasks complete successfully
        // Expected:
        //   - Task A correctly blocks on semaphore wait
        //   - Task B successfully releases semaphore
        //   - Task A resumes execution after semaphore release
        //   - Both tasks complete without errors
        
        osz_sem_t *sem = NULL;
        uint32_t res = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(res == OS_OK);

        st_sem_checker checker = {
            .sem = sem,
            .pend_cnt = 0,
            .post_cnt = 0,
            .test_end = 0,
        };

        // Create Task A (pend task)
        uint16_t task_pend_id = 0;
        void_t *pend_stack_addr = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t params_pend = {
            .name = "Task_A_Sync_Pend",
            .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
            .stack_base = (uintptr_t)pend_stack_addr,
            .stack_attr = STACK_MEM_DYNAMIC,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .thread = (task_callback_t)helper_task_sem_pend_entry,
            .data = (void_t *)(&checker),
        };
        res = osz_create_task(&task_pend_id, &params_pend);
        VERIFY(res == OS_OK);
        osz_task_resume(task_pend_id);

        // Verify Task A blocks on semaphore (pend_cnt should be 1 but test_end should be 0)
        while (checker.pend_cnt == 0) {
            osz_msleep(30);
        }
        
        VERIFY(checker.pend_cnt == 1);
        VERIFY(checker.post_cnt == 0);
        VERIFY(checker.test_end == 0); // Task A should be blocked
        VERIFY(checker.sem->field.sem == 0);

        // Create Task B (post task)
        uint16_t task_post_id = 0;
        void_t *post_stack_addr = osz_malloc(TASK_STACK_SIZE_DEFAULT);
        osz_task_params_t params_post = {
            .name = "Task_B_Sync_Post",
            .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
            .stack_attr = STACK_MEM_DYNAMIC,
            .stack_base = (uintptr_t)post_stack_addr,
            .stack_size = TASK_STACK_SIZE_DEFAULT,
            .thread = (task_callback_t)helper_task_sem_post_entry,
            .data = (void_t *)(&checker),
        };
        res = osz_create_task(&task_post_id, &params_post);
        VERIFY(res == OS_OK);
        osz_task_resume(task_post_id);

        // Wait for Task B to release semaphore
        while (checker.post_cnt == 0) {
            osz_msleep(30);
        }

        VERIFY(checker.pend_cnt == 1);
        VERIFY(checker.post_cnt == 1);

        // Wait for Task A to resume execution after semaphore release
        while (checker.test_end == 0) {
            osz_msleep(30);
        }
        VERIFY(checker.test_end == 1); // Task A resumed execution
        VERIFY(checker.sem->field.sem == 0);

        // Clean up
        res = osz_sem_detach(sem);
        VERIFY(res == OS_OK);
    }

    TEST("Test_1_4: Semaphore Zero Count Behavior") {
        // Preconditions: System initialized, available static semaphore object
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Attempt to acquire semaphore with zero timeout (should fail)
        //   3. Verify appropriate error code returned
        //   4. Release semaphore to make count positive
        //   5. Verify acquire now succeeds
        // Expected:
        //   - Acquire with zero count and zero timeout returns SEM_PEND_NO_WAIT_ERR
        //   - Release operation increases count to 1
        //   - Subsequent acquire operation succeeds
        //   - Error handling works correctly
        
        osz_sem_t *sem = NULL;
        uint32_t res = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(res == OS_OK);
        VERIFY(sem != NULL);

        // Step 2: Attempt to acquire semaphore with zero timeout (should fail)
        res = osz_sem_pend(sem, 0);
        VERIFY(res == SEM_PEND_NO_WAIT_ERR);
        VERIFY(sem->field.sem == 0); // Count should remain 0

        // Step 4: Release semaphore to make count positive
        res = osz_sem_post(sem);
        VERIFY(res == OS_OK);
        VERIFY(sem->field.sem == 1); // Count should become 1

        // Step 5: Verify acquire now succeeds
        res = osz_sem_pend(sem, 0);
        VERIFY(res == OS_OK);
        VERIFY(sem->field.sem == 0); // Count should become 0 after acquire

        // Clean up
        res = osz_sem_detach(sem);
        VERIFY(res == OS_OK);
    }

    TEST("Test_1_5: Semaphore Count Overflow Protection") {
        // Preconditions: System initialized, available static semaphore object
        // Steps:
        //   1. Initialize a semaphore with initial count close to maximum
        //   2. Attempt to release semaphore beyond maximum count
        //   3. Verify overflow protection works
        //   4. Verify appropriate error code returned
        // Expected:
        //   - Release beyond maximum count returns SEM_POST_NUM_OVER_ERR
        //   - Semaphore count remains at maximum value
        //   - System stability maintained during overflow attempt
        
        // Define SEM_MAX_COUNT to match the value in sem.c (0xffffffff)
        #define SEM_MAX_COUNT (0xffffffff)
        
        osz_sem_t *sem = NULL;
        uint32_t res = osz_sem_init(NULL, 0, SEM_MAX_COUNT, &sem);
        VERIFY(res == OS_OK);
        VERIFY(sem != NULL);

        // Verify initial count is SEM_MAX_COUNT
        VERIFY(sem->field.sem == SEM_MAX_COUNT);

        // Attempt to release semaphore beyond maximum count
        res = osz_sem_post(sem);
        VERIFY(res == SEM_POST_NUM_OVER_ERR);

        // Verify semaphore count remains at maximum value
        VERIFY(sem->field.sem == SEM_MAX_COUNT);

        // Clean up
        res = osz_sem_detach(sem);
        VERIFY(res == OS_OK);
    }
}
