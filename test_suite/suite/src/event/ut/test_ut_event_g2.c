/**
 * @file test_event.c
 * @brief Unit test cases group 2 for event module
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

#define OS_WAIT_FOREVER                     (0xFFFFFFFF)

static void_t setup(void_t) {}
static void_t teardown(void_t) {}

// =========================================================================
// Test Group 2: Event Read Tests
// =========================================================================
TEST_GROUP(ET_MODULE_EVENT, 2, "Event Read Tests", setup, teardown)
{
    TEST("Test_2_1: Normal Event Read (AND Operation, Events Exist)") {
        // Preconditions: Event initialized with specific event bits set
        // Steps:
        //   1. Call osz_events_read with AND operation and matching event set
        //   2. Check return status and output events
        // Expected:
        //   - Function returns OS_OK
        //   - Output events match the requested event set
        //   - Event bits remain unchanged (no CLEAN flag)
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        // Set specific event bits
        event_obj->field.events = 0x03; // Set bits 0 and 1
        
        uint32_t out_events = 0;
        result = osz_events_read(event_obj, 0x03, EVENT_FLAG_AND, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == OS_OK);
        VERIFY(out_events == 0x03);
        VERIFY(event_obj->field.events == 0x03); // Events remain unchanged
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_2_2: Normal Event Read (OR Operation, Events Exist)") {
        // Preconditions: Event initialized with specific event bits set
        // Steps:
        //   1. Call osz_events_read with OR operation and matching event set
        //   2. Check return status and output events
        // Expected:
        //   - Function returns OS_OK
        //   - Output events contain matching event bits
        //   - Event bits remain unchanged (no CLEAN flag)
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        // Set specific event bits
        event_obj->field.events = 0x05; // Set bits 0 and 2
        
        uint32_t out_events = 0;
        result = osz_events_read(event_obj, 0x04, EVENT_FLAG_OR, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == OS_OK);
        VERIFY(out_events == 0x04); // Only bit 2 matches (0x04)
        VERIFY(event_obj->field.events == 0x05); // Events remain unchanged
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_2_3: Event Read with Clean Flag") {
        // Preconditions: Event initialized with specific event bits set
        // Steps:
        //   1. Call osz_events_read with CLEAN flag and matching event set
        //   2. Check return status and event bits state
        // Expected:
        //   - Function returns OS_OK
        //   - Output events match the requested event set
        //   - Event bits are cleared after reading
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        // Set specific event bits
        event_obj->field.events = 0x07; // Set bits 0, 1 and 2
        
        uint32_t out_events = 0;
        result = osz_events_read(event_obj, 0x03, EVENT_FLAG_AND | EVENT_FLAG_CLEAN, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == OS_OK);
        VERIFY(out_events == 0x03);
        VERIFY(event_obj->field.events == 0x04); // Events cleared for bits 0 and 1, bit 2 remains
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_2_4: Event Read No Wait (Timeout Zero)") {
        // Preconditions: Event initialized without requested event bits
        // Steps:
        //   1. Call osz_events_read with timeout parameter 0
        //   2. Check return status
        // Expected:
        //   - Function immediately returns EVENT_READ_NO_WAIT_ERR
        //   - Current task does not enter waiting state
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        // Event has no requested bits set
        event_obj->field.events = 0x04; // Set bit 2, but we'll wait for bit 1
        
        uint32_t out_events = 0;
        result = osz_events_read(event_obj, 0x02, EVENT_FLAG_AND, 0, &out_events);
        
        VERIFY(result == EVENT_READ_NO_WAIT_ERR);
        VERIFY(event_obj->field.events == 0x04); // Events remain unchanged
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_2_5: Operation Flag Conflict Error Test") {
        // Preconditions: Event initialized, system running
        // Steps:
        //   1. Call osz_events_read with both AND and OR flags set
        //   2. Check return status
        // Expected:
        //   - Function returns EVENT_READ_OP_CONFLICT_ERR
        //   - No state change, no task blocking
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        uint32_t out_events = 0;
        result = osz_events_read(event_obj, 0x01, EVENT_FLAG_AND | EVENT_FLAG_OR, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == EVENT_READ_OP_CONFLICT_ERR);
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_2_6: Non-existent Operation Flag Error Test") {
        // Preconditions: Event initialized, system running
        // Steps:
        //   1. Call osz_events_read without AND or OR flags
        //   2. Check return status
        // Expected:
        //   - Function returns EVENT_READ_OP_NOT_EXSIT_ERR
        //   - No state change
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        uint32_t out_events = 0;
        // Call osz_events_read without AND or OR flags (operation flag = 0)
        result = osz_events_read(event_obj, 0x01, 0, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == EVENT_READ_OP_NOT_EXSIT_ERR);
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_2_7: Read NULL Event Object") {
        // Preconditions: No special preconditions
        // Steps:
        //   1. Call osz_events_read function, pass NULL as event parameter
        //   2. Check return status
        // Expected:
        //   - Function returns EVENT_READ_EVENTS_NULL_ERR
        //   - No state change, no task blocking
        
        // TODO: Implement test code here
        uint32_t out_events = 0;
        uint32_t result = osz_events_read(NULL, 0x01, EVENT_FLAG_AND, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == EVENT_READ_EVENTS_NULL_ERR);
    }

    TEST("Test_2_8: Read Non-Event Type Object") {
        // Preconditions: Prepare a non-event type object (e.g., task object)
        // Steps:
        //   1. Call osz_events_read function with non-event type object
        //   2. Check return status
        // Expected:
        //   - Function returns EVENT_READ_EVENTS_OBJ_NOT_IPC_ERR
        //   - No state change
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        // Modify the object type to non-event type (e.g., set to OSZ_MOD_TASK or other value)
        event_obj->attr.ipc_type = IPC_NONE_OBJECT; // Set to a non-event type
        
        uint32_t out_events = 0;
        result = osz_events_read(event_obj, 0x01, EVENT_FLAG_AND, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == EVENT_READ_NOT_EVENTS_TYPE_ERR);
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }

    TEST("Test_2_9: Read Unused Event Object") {
        // Preconditions: Create an event object marked as unused
        // Steps:
        //   1. Call osz_events_read function with unused event object
        //   2. Check return status
        // Expected:
        //   - Function returns EVENT_READ_EVENTS_OBJ_NOT_USED_ERR
        //   - No state change
        
        // TODO: Implement test code here
        osz_events_t *event_obj = NULL;
        uint8_t name[] = "test_event";
        uint8_t name_size = sizeof(name) - 1;
        uint32_t result = osz_events_init(name, name_size, &event_obj);
        VERIFY(result == OS_OK);
        VERIFY(event_obj != NULL);
        
        // Mark the event object as unused by setting ipc_obj_used to 0
        event_obj->attr.ipc_obj_used = 0;
        
        uint32_t out_events = 0;
        result = osz_events_read(event_obj, 0x01, EVENT_FLAG_AND, OS_WAIT_FOREVER, &out_events);
        
        VERIFY(result == EVENT_READ_EVENTS_OBJ_NOT_USED_ERR);
        
        result = osz_events_detach(event_obj);
        VERIFY(result == OS_OK);
    }
}
