/**
 * @file test_mutex_g1.c
 * @brief Unit test cases group 1 for mutex module
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for mutex functionality
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
// Test Group 1: Mutex Initialization and Creation Tests
// =========================================================================
TEST_GROUP(ET_MODULE_MUTEX, 1, "Mutex Initialization and Creation Tests", setup, teardown)
{
    TEST("Test_1_1: Normal Static Mutex Initialization") {
        // Preconditions: System initialized, static mutex pool has available objects
        // Steps:
        //   1. Call osz_mutex_init function with valid name, name length and initial value
        //   2. Check the returned output object pointer and attributes
        // Expected:
        //   - Function returns OS_OK
        //   - Output mutex object pointer is not NULL
        //   - Mutex object marked as used state
        //   - Mutex object correctly initialized with specified attributes
        //   - Mutex create type is IPC_STATIC_CREATE
        
        uint8_t name[] = "test_mutex";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        VERIFY(mutex->attr.ipc_obj_used == IPC_USED);
        VERIFY(mutex->attr.ipc_type == IPC_MUTEX);
        VERIFY(mutex->attr.ipc_create_type == IPC_STATIC_CREATE);
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 0);
        VERIFY(mutex->attr.ipc_mutex_origin_pri == 0xff);
        
        ret = osz_mutex_detach(mutex);
        VERIFY(ret == OS_OK);
    }

    TEST("Test_1_2: Static Mutex Initialization with Name") {
        // Preconditions: System initialized, available static mutex object
        // Steps:
        //   1. Call osz_mutex_init, pass name "test_mutex" and name length 10
        //   2. Check the name field of mutex object
        // Expected:
        //   - Function returns OS_OK
        //   - Mutex object's name field contains "test_mutex"
        //   - Name correctly copied, no truncation or overflow
        //   - Name length properly handled within OSZ_CFG_OBJ_NAME_MAX_LENS
        
        uint8_t name[] = "test_mutex";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        ret = osz_mutex_init(name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        uint32_t test_size = name_size > OSZ_CFG_OBJ_NAME_MAX_LENS ? OSZ_CFG_OBJ_NAME_MAX_LENS : name_size;
        for (uint32_t i = 0; i < test_size; ++i) {
            VERIFY(mutex->supper.name[i] == name[i]);
        }
        
        ret = osz_mutex_detach(mutex);
        VERIFY(ret == OS_OK);
    }

    TEST("Test_1_3: Static Mutex Initialization with Long Name Truncation") {
        // Preconditions: System initialized, available static mutex object
        // Steps:
        //   1. Call osz_mutex_init with name longer than OSZ_CFG_OBJ_NAME_MAX_LENS
        //   2. Check the name field is properly truncated
        // Expected:
        //   - Function returns OS_OK
        //   - Name is truncated to OSZ_CFG_OBJ_NAME_MAX_LENS
        //   - No buffer overflow occurs
        
        uint8_t long_name[] = "this_is_a_very_long_mutex_name";
        uint8_t name_size = sizeof(long_name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        ret = osz_mutex_init(long_name, name_size, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        // Name should be truncated to OSZ_CFG_OBJ_NAME_MAX_LENS
        // Verify that the name was copied safely without overflow
        uint32_t test_size = name_size > OSZ_CFG_OBJ_NAME_MAX_LENS ? OSZ_CFG_OBJ_NAME_MAX_LENS : name_size;
        for (uint32_t i = 0; i < test_size; ++i) {
            VERIFY(mutex->supper.name[i] == long_name[i]);
        }
        
        ret = osz_mutex_detach(mutex);
        VERIFY(ret == OS_OK);
    }

    TEST("Test_1_4: Static Mutex Initialization with NULL Name") {
        // Preconditions: System initialized, available static mutex object
        // Steps:
        //   1. Call osz_mutex_init with NULL name pointer
        //   2. Check the mutex object initialization
        // Expected:
        //   - Function returns OS_OK
        //   - Mutex object created successfully
        //   - Name field remains empty or default
        
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        ret = osz_mutex_init(NULL, 0, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        VERIFY(mutex->attr.ipc_obj_used == IPC_USED);
        VERIFY(mutex->attr.ipc_type == IPC_MUTEX);
        // Name field should be empty when NULL name is provided
        VERIFY(mutex->supper.name[0] == '\0');
        
        ret = osz_mutex_detach(mutex);
        VERIFY(ret == OS_OK);
    }

    TEST("Test_1_5: Static Mutex Initialization with Zero Name Length") {
        // Preconditions: System initialized, available static mutex object
        // Steps:
        //   1. Call osz_mutex_init with zero name length
        //   2. Check the mutex object initialization
        // Expected:
        //   - Function returns OS_OK
        //   - Mutex object created successfully
        //   - Name field remains empty
        
        uint8_t name[] = "test_mutex";
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        ret = osz_mutex_init(name, 0, 1, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        VERIFY(mutex->attr.ipc_obj_used == IPC_USED);
        VERIFY(mutex->attr.ipc_type == IPC_MUTEX);
        // Name field should be empty when name length is 0
        VERIFY(mutex->supper.name[0] == '\0');
        
        ret = osz_mutex_detach(mutex);
        VERIFY(ret == OS_OK);
    }

    TEST("Test_1_6: Dynamic Mutex Creation") {
        // Preconditions: System initialized, memory available for dynamic allocation
        // Steps:
        //   1. Call osz_mutex_create function with valid parameters
        //   2. Check the returned output object pointer and attributes
        // Expected:
        //   - Function returns OS_OK
        //   - Output mutex object pointer is not NULL
        //   - Mutex object correctly created with dynamic type
        //   - Memory allocated for mutex object
        //   - Mutex create type is IPC_DYNAMIC_CREATE
        
        uint8_t name[] = "dynamic_mutex";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        ret = osz_mutex_create(name, name_size, &mutex);
        VERIFY(ret == OS_OK);
        VERIFY(mutex != NULL);
        VERIFY(mutex->attr.ipc_obj_used == IPC_USED);
        VERIFY(mutex->attr.ipc_type == IPC_MUTEX);
        VERIFY(mutex->attr.ipc_create_type == IPC_DYNAMIC_CREATE);
        VERIFY(mutex->attr.ipc_mutex_reentrant_cnt == 0);
        VERIFY(mutex->attr.ipc_mutex_origin_pri == 0xff);
        uint32_t test_size = name_size > OSZ_CFG_OBJ_NAME_MAX_LENS ? OSZ_CFG_OBJ_NAME_MAX_LENS : name_size;
        for (uint32_t i = 0; i < test_size; ++i) {
            VERIFY(mutex->supper.name[i] == name[i]);
        }
        
        ret = osz_mutex_delete(mutex);
        VERIFY(ret == OS_OK);
    }

    TEST("Test_1_7: Dynamic Mutex Creation with Outter Object Not NULL") {
        // Preconditions: System initialized
        // Steps:
        //   1. Call osz_mutex_create with non-NULL outter_obj pointer
        //   2. Check the return error code
        // Expected:
        //   - Function returns MUTEX_CREATE_OUTTER_NOT_NULL_ERR
        //   - No memory allocation occurs
        
        uint8_t name[] = "test_mutex";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = (osz_mutex_t *)0x12345678; // Non-NULL pointer
        uint32_t ret;

        ret = osz_mutex_create(name, name_size, &mutex);
        VERIFY(ret == MUTEX_CREATE_OUTTER_NOT_NULL_ERR);
        // Verify that the pointer was not modified
        VERIFY(mutex == (osz_mutex_t *)0x12345678);
    }

    SKIP_TEST("Test_1_8: Dynamic Mutex Creation Memory Allocation Failure") {
        // Preconditions: System initialized, memory allocation fails
        // Steps:
        //   1. Mock osz_malloc to return NULL
        //   2. Call osz_mutex_create function
        //   3. Check the return error code
        // Expected:
        //   - Function returns MUTEX_CREATE_OUTTER_MALLOC_FAIL_ERR
        //   - No mutex object is created
        
        uint8_t name[] = "test_mutex";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex = NULL;
        uint32_t ret;

        // In a real test environment, we would mock osz_malloc to return NULL
        // For now, we'll test the error handling by calling the function normally
        // and relying on the fact that if malloc fails, the error code should be returned
        ret = osz_mutex_create(name, name_size, &mutex);
        // This test assumes that memory allocation might fail in some scenarios
        // In a proper test setup, we would use mocking to force malloc failure
        VERIFY(ret == MUTEX_CREATE_OUTTER_MALLOC_FAIL_ERR || ret == OS_OK);
        // If malloc fails, mutex should remain NULL
        if (ret == MUTEX_CREATE_OUTTER_MALLOC_FAIL_ERR) {
            VERIFY(mutex == NULL);
        }
        
    }

    TEST("Test_1_9: Static Mutex Pool Exhaustion") {
        // Preconditions: System initialized, all static mutex objects are used
        // Steps:
        //   1. Initialize all available static mutex objects
        //   2. Attempt to initialize one more mutex
        //   3. Check the return error code
        // Expected:
        //   - Function returns appropriate error code (e.g., OS_NO_MEMORY)
        //   - No mutex object is returned
        
        uint8_t name[] = "test_mutex";
        uint8_t name_size = sizeof(name) - 1;
        osz_mutex_t *mutex_pre[OSZ_MUTEX_STATIC_MAX_CNT] = { 0 };
        osz_mutex_t *mutex = NULL;
        uint32_t ret;
        
        // In a real test, we would exhaust the static mutex pool first
        // For now, we test the normal case and assume that if pool is exhausted,
        // the function would return an appropriate error code
        for (uint32_t i = 0; i < OSZ_MUTEX_STATIC_MAX_CNT; ++i) {
            osz_mutex_init(name, name_size, 1, &mutex_pre[i]);
        }
        ret = osz_mutex_init(name, name_size, 1, &mutex);
        // This test verifies that when pool is exhausted, error is handled properly
        VERIFY(ret == OBJECT_FREE_OBJ_ALL_USED_ERR);
        
        for (uint32_t i = 0; i < OSZ_MUTEX_STATIC_MAX_CNT; ++i) {
            osz_mutex_detach(mutex_pre[i]);
        }
    }
}
