/**
 * @file test_event.c
 * @brief Unit test cases group 1 for event module
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for event functionality
 * 
 * Coverage targets:
 * - Statement coverage: 100%
 * - Branch coverage: 100%  
 * - Function coverage: 100%
 */

#include "event.h"
#include "et.h"

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 1: Event Initialization Tests
// =========================================================================
TEST_GROUP(ET_MODULE_EVENT, 1, "Event Initialization Tests", setup, teardown)
{
    TEST("Test_1_1: Normal Static Event Initialization") {
        // Preconditions: System initialized, static event pool has available objects
        // Steps:
        //   1. Call osz_events_init function with valid name, name length
        //   2. Check the returned output object pointer
        // Expected:
        //   - Function returns OS_OK
        //   - Output event object pointer is not NULL
        //   - Event object correctly initialized with default values
        //   - Event object marked as used state
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        VERIFY(event_obj->attr.ipc_obj_used == IPC_USED);
        VERIFY(event_obj->attr.ipc_type == IPC_EVENTS);
        VERIFY(event_obj->field.events == 0);
        VERIFY(DLINK_EMPTY(&event_obj->pend_list));

        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_1_2: Event Initialization with Name") {
        // Preconditions: System initialized, available static event object
        // Steps:
        //   1. Call osz_events_init, pass name "test_event" and name length 10
        //   2. Check the name field of event object
        // Expected:
        //   - Function returns OS_OK
        //   - Event object's name field contains "test_event"
        //   - Name correctly copied, no truncation or overflow
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = 10; // Use the specified name length
        
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        for (uint32_t i = 0; i < OSZ_CFG_OBJ_NAME_MAX_LENS; ++i) {
            VERIFY(event_obj->supper.name[i] == name[i]);
        }
        VERIFY(event_obj->attr.ipc_obj_used == IPC_USED);
        VERIFY(event_obj->attr.ipc_type == IPC_EVENTS);
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }
}
