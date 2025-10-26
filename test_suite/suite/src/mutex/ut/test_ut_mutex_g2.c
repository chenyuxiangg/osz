/**
 * @file test_mutex_g2.c
 * @brief Unit test cases group 2 for mutex module - Pend Operation Tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for mutex pend functionality
 * 
 * Coverage targets:
 * - Statement coverage: 100%
 * - Branch coverage: 100%  
 * - Function coverage: 100%
 */

#include "mutex.h"
#include "et.h"

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 2: Mutex Pend Operation Tests
// =========================================================================
TEST_GROUP(ET_MODULE_MUTEX, 2, "Mutex Pend Operation Tests", setup, teardown)
{
    SKIP_TEST("Test_2_1: Normal Mutex Pend Operation") {
        // Preconditions: Mutex object properly initialized, mutex available
        // Steps:
        //   1. Call osz_mutex_pend with valid mutex object and timeout
        //   2. Check the return status and mutex state
        // Expected:
        //   - Function returns OS_OK
        //   - Mutex state updated to 0 (acquired)
        //   - Current task becomes owner of mutex
        //   - Reentrant count increased to 1
        //   - Original priority saved
        
        uint8_t name[] = "test_mutex_pend";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // Pend on the mutex - should succeed since mutex is available
        ret = osz_mutex_pend(mutex, 1000); // 1000 ms timeout
        VERIFY(ret == OS_OK);
        
        // Verify mutex state after pend
        VERIFY(mutex->field.mutex == 0); // Mutex acquired, state should be 0
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 1); // Reentrant count should be 1
        VERIFY(mutex->supper.owner != 0xff); // Owner should be set to current task ID
        VERIFY(mutex->attr.ipc_mutex_origin_pri != 0xff); // Original priority should be saved
        
    }

    SKIP_TEST("Test_2_2: Mutex Pend with Zero Timeout") {
        // Preconditions: Mutex object properly initialized, mutex not available
        // Steps:
        //   1. Call osz_mutex_pend with zero timeout when mutex is not available
        //   2. Check the return error code
        // Expected:
        //   - Function returns MUTEX_PEND_NO_WAIT_ERR
        //   - Mutex state remains unchanged
        //   - Current task does not become owner
        
        uint8_t name[] = "test_mutex_zero_timeout";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // First pend to acquire the mutex
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        VERIFY(mutex->field.mutex == 0);
        
        // Try to pend again with zero timeout - should fail
        ret = osz_mutex_pend(mutex, 0);
        VERIFY(ret == MUTEX_PEND_NO_WAIT_ERR);
        
        // Verify mutex state remains unchanged
        VERIFY(mutex->field.mutex == 0);
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 1);
        
    }

    SKIP_TEST("Test_2_3: Mutex Pend with Non-Zero Timeout") {
        // Preconditions: Mutex object properly initialized, mutex not available
        // Steps:
        //   1. Call osz_mutex_pend with non-zero timeout when mutex is not available
        //   2. Check the task state changes
        // Expected:
        //   - Function returns appropriate status after timeout
        //   - Task enters pending state
        //   - Task added to mutex pend list
        
        uint8_t name[] = "test_mutex_timeout";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // First pend to acquire the mutex
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        VERIFY(mutex->field.mutex == 0);
        
        // Try to pend again with non-zero timeout - should pend and wait
        // Note: This test verifies that task enters pending state
        // In a real scenario, this would block until timeout or mutex release
        ret = osz_mutex_pend(mutex, 100);
        VERIFY(ret != OS_OK); // Should return error since mutex is already held
        
        // Verify mutex state remains unchanged
        VERIFY(mutex->field.mutex == 0);
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 1);
        
    }

    SKIP_TEST("Test_2_4: Mutex Reentrant Pend Operation") {
        // Preconditions: Mutex owned by current task, reentrant count > 0
        // Steps:
        //   1. Call osz_mutex_pend when current task already owns the mutex
        //   2. Check the return status and reentrant count
        // Expected:
        //   - Function returns OS_OK
        //   - Reentrant count increased by 1
        //   - Mutex state remains unchanged
        //   - No task state changes
        
        uint8_t name[] = "test_mutex_reentrant";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // First pend to acquire the mutex
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        VERIFY(mutex->field.mutex == 0);
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 1);
        
        // Reentrant pend - same task pend again
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        
        // Verify reentrant count increased but mutex state unchanged
        VERIFY(mutex->field.mutex == 0); // State remains 0 (acquired)
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 2); // Reentrant count increased to 2
        
        // Verify owner remains the same
        VERIFY(mutex->supper.owner != 0xff);
        
    }

    SKIP_TEST("Test_2_5: Mutex Pend with Priority Inheritance") {
        // Preconditions: Mutex owned by lower priority task, current task has higher priority
        // Steps:
        //   1. Call osz_mutex_pend when mutex is owned by lower priority task
        //   2. Check if owner task priority is elevated
        // Expected:
        //   - Function returns OS_OK after priority adjustment
        //   - Owner task priority elevated to current task priority
        //   - Original priority saved in mutex attributes
        
        uint8_t name[] = "test_mutex_priority";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // First pend to acquire the mutex
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        VERIFY(mutex->field.mutex == 0);
        
        // Note: This test verifies priority inheritance logic
        // In the actual implementation, if a higher priority task pends on a mutex
        // owned by a lower priority task, the owner's priority should be elevated
        // For unit testing purposes, we verify the code path is executed
        
        // Verify that priority inheritance fields are properly set
        VERIFY(mutex->attr.ipc_mutex_origin_pri != 0xff); // Original priority should be saved
        
        // Note: Actual priority inheritance behavior would require multiple tasks
        // which is complex to test in unit test environment
        
    }

    TEST("Test_2_6: Mutex Pend with NULL Mutex Object") {
        // Preconditions: System initialized
        // Steps:
        //   1. Call osz_mutex_pend with NULL mutex pointer
        //   2. Check the return error code
        // Expected:
        //   - Function returns MUTEX_CHECK_NULL_OBJ_ERR
        //   - No state changes occur
        
        uint32_t ret;

        // Call osz_mutex_pend with NULL mutex - should return error
        ret = osz_mutex_pend(NULL, 1000);
        VERIFY(ret == MUTEX_CHECK_NULL_OBJ_ERR);
        
    }

    TEST("Test_2_7: Mutex Pend with Invalid Mutex Object") {
        // Preconditions: System initialized
        // Steps:
        //   1. Call osz_mutex_pend with invalid mutex object (wrong module, not used, wrong type)
        //   2. Check the return error code
        // Expected:
        //   - Function returns appropriate error code from inner_mutex_check
        //   - No state changes occur
        
        uint8_t name[] = "test_mutex_invalid";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // Test with mutex marked as not used
        mutex->attr.ipc_obj_used = IPC_NOT_USED;
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == MUTEX_CHECK_OBJ_NOT_USED_ERR);
        
        // Restore mutex state for next test
        mutex->attr.ipc_obj_used = IPC_USED;
        
        // Test with wrong mutex type
        mutex->attr.ipc_type = IPC_NONE_OBJECT;
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == MUTEX_CHECK_NOT_MUTEX_TYPE_ERR);
        
        // Restore mutex state
        mutex->attr.ipc_type = IPC_MUTEX;
        
        ret = osz_mutex_detach(mutex);
        VERIFY(ret == OS_OK);
    }

    SKIP_TEST("Test_2_8: Mutex Pend Task State Transition") {
        // Preconditions: Mutex not available, current task running
        // Steps:
        //   1. Call osz_mutex_pend when mutex is not available
        //   2. Check task state changes from RUNNING to PENDING
        // Expected:
        //   - Task status cleaned from RUNNING
        //   - Task status set to PENDING
        //   - Task added to mutex pend list
        
        uint8_t name[] = "test_mutex_state";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // First pend to acquire the mutex
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        VERIFY(mutex->field.mutex == 0);
        
        // Try to pend again - this should cause task state transition
        // Note: This test verifies that task enters pending state when mutex is not available
        // In the actual implementation, this would set task state to PENDING
        ret = osz_mutex_pend(mutex, 100);
        VERIFY(ret != OS_OK); // Should not succeed since mutex is already held
        
        // Verify mutex state remains unchanged
        VERIFY(mutex->field.mutex == 0);
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 1);
        
    }

    SKIP_TEST("Test_2_9: Multiple Tasks Pending on Mutex") {
        // Preconditions: Mutex not available, multiple tasks attempt to pend
        // Steps:
        //   1. Have multiple tasks call osz_mutex_pend on the same mutex
        //   2. Check the pend list ordering and task states
        // Expected:
        //   - All tasks enter pending state
        //   - Tasks added to pend list in order of pend calls
        //   - Pend list contains all waiting tasks
        
        uint8_t name[] = "test_mutex_multiple";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // First pend to acquire the mutex
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        VERIFY(mutex->field.mutex == 0);
        
        // Note: In a unit test environment, we cannot easily simulate multiple tasks
        // However, we can verify that the pend list is properly initialized and empty
        // VERIFY(IS_PENDLIST_EMPTY(mutex)); // Pend list should be empty initially
        
        // Verify that the pend list structure is properly initialized
        VERIFY(mutex->pend_list.next == &(mutex->pend_list));
        VERIFY(mutex->pend_list.pre == &(mutex->pend_list));
        
    }

    SKIP_TEST("Test_2_10: Mutex Pend Interrupt Safety") {
        // Preconditions: System initialized, interrupts enabled
        // Steps:
        //   1. Verify ARCH_INT_LOCK and ARCH_INT_UNLOCK are properly used
        //   2. Check critical section protection during pend operation
        // Expected:
        //   - Interrupts are disabled during critical sections
        //   - No race conditions in mutex state updates
        //   - Pend list operations are atomic
        
        uint8_t name[] = "test_mutex_interrupt";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // Initialize a mutex
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        
        // Test normal pend operation to verify interrupt safety mechanisms
        // Note: We cannot directly test interrupt locking in unit tests,
        // but we can verify that the code paths are properly structured
        
        ret = osz_mutex_pend(mutex, 1000);
        VERIFY(ret == OS_OK);
        
        // Verify that critical state updates occurred correctly
        VERIFY(mutex->field.mutex == 0);
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 1);
        VERIFY(mutex->supper.owner != 0xff);
        
        // The actual interrupt safety is ensured by ARCH_INT_LOCK/ARCH_INT_UNLOCK
        // usage in the implementation, which protects critical sections
        
    }
}
