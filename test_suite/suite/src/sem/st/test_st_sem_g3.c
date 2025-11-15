/**
 * @file test_st_sem_g3.c
 * @brief System test cases group 3 for sem module - Semaphore and Interrupt Interaction Tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for semaphore and interrupt interaction
 * 
 * Coverage targets:
 * - Function coverage: 100%
 * - Scenario coverage: Interrupt context scenarios
 * - Module interaction: Interrupt handling, task scheduling, and IPC
 */

#include "mem.h"
#include "sem.h"
#include "et.h"
#include "clint.h"
#include "test_st_sem_g3_helper.h"

#define TASK_STACK_SIZE_DEFAULT     (0x2000)

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 3: Semaphore and Interrupt Interaction Tests
// =========================================================================
TEST_GROUP(ET_MODULE_SEM_ST, 3, "Semaphore and Interrupt Interaction Tests", setup, teardown)
{
    TEST("Test_3_1: Semaphore Release from Interrupt Context") {
        // Preconditions: System initialized, task waiting on semaphore, interrupt configured
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Create a task that waits on the semaphore
        //   3. Trigger an interrupt that releases the semaphore
        //   4. Verify the waiting task is woken up by the interrupt
        //   5. Verify task resumes execution after interrupt
        // Expected:
        //   - Interrupt handler can successfully release semaphore
        //   - Waiting task is woken up from interrupt context
        //   - Task resumes execution after interrupt handler completes
        //   - No data corruption or race conditions occur

        helper_software_interrupt_init((interrupt_callback)helper_software_int_handler_3_1);
        osz_sem_t *sem = NULL;
        uint32_t res = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(res == OS_OK);
        VERIFY(sem != NULL);
        st_sem_checker checker = {
            .pend_cnt = 0,
            .post_cnt = 0,
            .sem = sem,
            .test_end = 0,
            .wake_task = 0,
        };

        res = helper_task_create("test_3_1", PRIORITY_FOR_TEST_SEM_DEFAULT, TASK_STACK_SIZE_DEFAULT, (task_callback_t)helper_3_1_task_entry, (void_t *)&checker);
        VERIFY(res == OS_OK);

        osz_msleep(0x200);
        VERIFY(checker.pend_cnt == 1);

        helper_software_interrupt_trigger();

        VERIFY(checker.post_cnt == 1);
        VERIFY(checker.test_end == 1);
        VERIFY(checker.sem->field.sem == 0);

        helper_software_interrupt_deinit();
        osz_sem_detach(sem);
    }

    TEST("Test_3_2: Semaphore Wait Queue Management during Interrupts") {
        // Preconditions: System initialized, multiple tasks waiting on semaphore, interrupt configured
        // Steps:
        //   1. Initialize a semaphore with initial count 0
        //   2. Create multiple tasks waiting on the semaphore
        //   3. Trigger an interrupt that releases the semaphore multiple times
        //   4. Verify waiting tasks are woken in correct order
        //   5. Verify semaphore wait list is correctly managed during interrupts
        // Expected:
        //   - Interrupt handler correctly manages semaphore wait list
        //   - Tasks are woken in appropriate order (priority-based if applicable)
        //   - No tasks are missed or incorrectly woken
        
        osz_sem_t *sem = NULL;
        uint32_t res = osz_sem_init(NULL, 0, 0, &sem);
        VERIFY(res == OS_OK);
        VERIFY(sem != NULL);
        st_sem_checker checker = {
            .pend_cnt = 0,
            .post_cnt = 0,
            .sem = sem,
            .test_end = 0,
            .wake_task = 0,
        };

        helper_software_interrupt_init((interrupt_callback)helper_software_int_handler_3_2);

        res = helper_task_create("test_3_2_task1", PRIORITY_FOR_TEST_SEM_DEFAULT, TASK_STACK_SIZE_DEFAULT, (task_callback_t)helper_3_2_task_entry, (void_t *)&checker);
        VERIFY(res == OS_OK);

        osz_msleep(0x100);
        VERIFY(checker.pend_cnt == 1);

        res = helper_task_create("test_3_2_task2", PRIORITY_FOR_TEST_SEM_DEFAULT, TASK_STACK_SIZE_DEFAULT, (task_callback_t)helper_3_2_task_entry, (void_t *)&checker);
        VERIFY(res == OS_OK);
        osz_msleep(0x100);
        VERIFY(checker.pend_cnt == 2);

        res = helper_task_create("test_3_2_task3", PRIORITY_FOR_TEST_SEM_DEFAULT, TASK_STACK_SIZE_DEFAULT, (task_callback_t)helper_3_2_task_entry, (void_t *)&checker);
        VERIFY(res == OS_OK);
        osz_msleep(0x100);
        VERIFY(checker.pend_cnt == 3);

        helper_software_interrupt_trigger();

        VERIFY(checker.post_cnt == 2);
        VERIFY(checker.sem->field.sem == 0);

        helper_software_interrupt_trigger();

        VERIFY(checker.post_cnt == 3);
        VERIFY(checker.sem->field.sem == 1);

        helper_software_interrupt_deinit();
        osz_sem_detach(sem);
    }

    SKIP_TEST("Test_3_3: Nested Interrupts and Semaphore Operations") {
        // Preconditions: System initialized, nested interrupts enabled, semaphore initialized
        // Steps:
        //   1. Configure nested interrupt scenario
        //   2. Perform semaphore operations from nested interrupt context
        //   3. Verify semaphore behavior is correct under nested interrupts
        //   4. Verify system stability and no stack overflow occurs
        // Expected:
        //   - Semaphore operations work correctly in nested interrupt context
        //   - System remains stable with no stack overflow or corruption
        //   - Interrupt nesting depth does not affect semaphore functionality
        
        /* TODO: Current don't support nested interrupt. */
    }
}
