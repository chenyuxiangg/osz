/**
 * @file test_sem.c
 * @brief Unit test cases group 1 for sem module
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
// Test Group 1: Semaphore Initialization Tests
// =========================================================================
TEST_GROUP(ET_MODULE_SEM, 1, "Semaphore Initialization Tests", setup, teardown)
{
    TEST("Test_1_1: Normal Static Semaphore Initialization") {
        // Preconditions: System initialized, static semaphore pool has available objects
        // Steps:
        //   1. Call osz_sem_init function with valid name, name length and initial count 5
        //   2. Check the returned output object pointer
        // Expected:
        //   - Function returns OS_OK
        //   - Output semaphore object pointer is not NULL
        //   - Semaphore count value is 5
        //   - Semaphore object marked as used state
        
        osz_sem_t *pstSemaphore = NULL;
        uint8_t arrName[] = "test_sem";
        uint8_t u8NameSize = 8;
        uint32_t u32InitialCount = 5;
        uint32_t u32Ret = OS_OK;
        
        // Step 1: Call osz_sem_init function
        u32Ret = osz_sem_init(arrName, u8NameSize, u32InitialCount, &pstSemaphore);
        
        // Step 2: Check the returned output object pointer and status
        VERIFY(u32Ret == OS_OK);
        VERIFY(pstSemaphore != NULL);
        VERIFY(pstSemaphore->field.sem == u32InitialCount);
        VERIFY(pstSemaphore->attr.ipc_obj_used == IPC_USED);
        
        // Cleanup: detach the semaphore to avoid affecting other tests
        if (pstSemaphore != NULL) {
            u32Ret = osz_sem_detach(pstSemaphore);
            VERIFY(u32Ret == OS_OK);
        }
    }

    TEST("Test_1_2: Semaphore Initialization with Name") {
        // Preconditions: System initialized, available static semaphore object
        // Steps:
        //   1. Call osz_sem_init, pass name "test_sem" and name length 8
        //   2. Check the name field of semaphore object
        // Expected:
        //   - Function returns OS_OK
        //   - Semaphore object's name field contains "test_sem"
        //   - Name correctly copied, no truncation or overflow
        
        osz_sem_t *pstSemaphore = NULL;
        uint8_t arrName[] = "test_sem";
        uint8_t u8NameSize = 8;
        uint32_t u32InitialCount = 1;
        uint32_t u32Ret = OS_OK;
        
        // Step 1: Call osz_sem_init function with name
        u32Ret = osz_sem_init(arrName, u8NameSize, u32InitialCount, &pstSemaphore);
        
        // Step 2: Check the name field of semaphore object
        VERIFY(u32Ret == OS_OK);
        VERIFY(pstSemaphore != NULL);
        for (uint32_t i = 0; i < u8NameSize; ++i) {
            VERIFY(pstSemaphore->supper.name[i] == arrName[i]);
        }
        
        // Cleanup: detach the semaphore to avoid affecting other tests
        if (pstSemaphore != NULL) {
            u32Ret = osz_sem_detach(pstSemaphore);
            VERIFY(u32Ret == OS_OK);
        }
    }
}
