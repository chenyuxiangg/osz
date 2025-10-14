/**
 * @file test_sem.c
 * @brief Unit test cases group 2 for sem module
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for sem functionality
 * 
 * Coverage targets:
 * - Statement coverage: 100%
 * - Branch coverage: 100%  
 * - Function coverage: 100%
 */

#include "sem.h"
#include "et.h"

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 2: Semaphore Pend Tests
// =========================================================================
TEST_GROUP(ET_MODULE_SEM, 2, "Semaphore Pend Tests", setup, teardown)
{
    TEST("Test_2_1: Normal Semaphore Pend (Available Resources)") {
        // Preconditions: Semaphore initialized with initial count 3
        // Steps:
        //   1. Call osz_sem_pend function with timeout parameter 1000
        //   2. Check return status and semaphore count value
        // Expected:
        //   - Function immediately returns OS_OK
        //   - Semaphore count value decreases to 2
        //   - Current task status remains running state
        
        // TODO: Implement test code here
        osz_sem_t *pstSemaphore = NULL;
        uint8_t arrName[] = "test_sem";
        uint8_t u8NameSize = 8;
        uint32_t u32InitialCount = 3;
        uint32_t u32Timeout = 1000;
        uint32_t u32Ret = OS_OK;
        
        // Initialize semaphore with count 3 and valid name
        u32Ret = osz_sem_init(arrName, u8NameSize, u32InitialCount, &pstSemaphore);
        VERIFY(u32Ret == OS_OK);
        VERIFY(pstSemaphore != NULL);
        VERIFY(pstSemaphore->field.sem == u32InitialCount);
        
        // Step 1: Call osz_sem_pend function with timeout parameter 1000
        u32Ret = osz_sem_pend(pstSemaphore, u32Timeout);
        
        // Step 2: Check return status and semaphore count value
        VERIFY(u32Ret == OS_OK);
        VERIFY(pstSemaphore->field.sem == (u32InitialCount - 1));
        
        // Cleanup: delete the task and detach the semaphore
        if (pstSemaphore != NULL) {
            osz_sem_detach(pstSemaphore);
        }
    }

    TEST("Test_2_2: Zero Timeout Pend (No Available Resources)") {
        // Preconditions: Semaphore initialized with initial count 0
        // Steps:
        //   1. Call osz_sem_pend function with timeout parameter 0
        //   2. Check return status
        // Expected:
        //   - Function immediately returns SEM_PEND_NO_WAIT_ERR
        //   - Semaphore count value remains 0
        //   - Current task does not enter waiting state
        
        // TODO: Implement test code here
        osz_sem_t *pstSemaphore = NULL;
        uint8_t arrName[] = "test_sem";
        uint8_t u8NameSize = 8;
        uint32_t u32InitialCount = 0;
        uint32_t u32Timeout = 0;
        uint32_t u32Ret = OS_OK;

        // Initialize semaphore with count 0
        u32Ret = osz_sem_init(arrName, u8NameSize, u32InitialCount, &pstSemaphore);
        VERIFY(u32Ret == OS_OK);
        VERIFY(pstSemaphore != NULL);
        VERIFY(pstSemaphore->field.sem == u32InitialCount);

        // Step 1: Call osz_sem_pend function with timeout 0
        u32Ret = osz_sem_pend(pstSemaphore, u32Timeout);

        // Step 2: Check return status and semaphore count value
        VERIFY(u32Ret == SEM_PEND_NO_WAIT_ERR);
        VERIFY(pstSemaphore->field.sem == u32InitialCount); // Count should remain 0

        // Cleanup: detach the semaphore
        if (pstSemaphore != NULL) {
            osz_sem_detach(pstSemaphore);
        }
    }

    TEST("Test_2_3: Pend with NULL Pointer Semaphore") {
        // Preconditions: No special preconditions
        // Steps:
        //   1. Call osz_sem_pend function, pass NULL as semaphore parameter
        //   2. Check return status
        // Expected:
        //   - Function returns SEM_PEND_NULL_OBJ_ERR
        //   - No state change, no task blocking
        
        // TODO: Implement test code here
        uint32_t u32Timeout = 1000;
        uint32_t u32Ret = OS_OK;

        // Step 1: Call osz_sem_pend function with NULL semaphore parameter
        u32Ret = osz_sem_pend(NULL, u32Timeout);

        // Step 2: Check return status
        VERIFY(u32Ret == SEM_PEND_NULL_OBJ_ERR);
    }

    TEST("Test_2_4: Pend with Non-Semaphore Type Object") {
        // Preconditions: Prepare a non-semaphore type object (e.g., task object)
        // Steps:
        //   1. Call osz_sem_pend function with non-semaphore type object
        //   2. Check return status
        // Expected:
        //   - Function returns SEM_PEND_OBJ_NOT_SEM_ERR
        //   - No state change
        
        // TODO: Implement test code here
        uint32_t u32Timeout = 1000;
        uint32_t u32Ret = OS_OK;
        osz_obj_t stObject = {0};  // Mock task object (non-semaphore type)

        // Step 1: Call osz_sem_pend function with non-semaphore type object
        // Cast the task object to osz_sem_t* to simulate passing wrong object type
        u32Ret = osz_sem_pend((osz_sem_t*)&stObject, u32Timeout);

        // Step 2: Check return status
        VERIFY(u32Ret == SEM_PEND_OBJ_NOT_SEM_ERR);
    }

    TEST("Test_2_5: Pend with Unused Semaphore Object") {
        // Preconditions: Create a semaphore object marked as unused
        // Steps:
        //   1. Call osz_sem_pend function with unused semaphore object
        //   2. Check return status
        // Expected:
        //   - Function returns SEM_PEND_OBJ_NOT_USED_ERR
        //   - No state change
        
        // TODO: Implement test code here
        osz_sem_t *pstSemaphore = NULL;
        uint8_t arrName[] = "test_sem";
        uint8_t u8NameSize = 8;
        uint32_t u32InitialCount = 1;
        uint32_t u32Timeout = 1000;
        uint32_t u32Ret = OS_OK;

        // Initialize semaphore and then detach it to make it unused
        u32Ret = osz_sem_init(arrName, u8NameSize, u32InitialCount, &pstSemaphore);
        VERIFY(u32Ret == OS_OK);
        VERIFY(pstSemaphore != NULL);
        
        // Detach the semaphore to mark it as unused
        u32Ret = osz_sem_detach(pstSemaphore);
        VERIFY(u32Ret == OS_OK);

        // Step 1: Call osz_sem_pend function with unused semaphore object
        u32Ret = osz_sem_pend(pstSemaphore, u32Timeout);

        // Step 2: Check return status
        VERIFY(u32Ret == SEM_PEND_OBJ_NOT_USED_ERR);
    }
}
