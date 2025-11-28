#include "test_st_event_g1_helper.h"

void_t helper_task_event_read_entry(void_t *data)
{
    if (data == NULL) {
        return;
    }
    st_event_checker *checker = (st_event_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->read_cnt = 1;
    ARCH_INT_UNLOCK(intsave);
    
    uint32_t read_events = 0;
    uint32_t res = osz_events_read(checker->events, checker->expected_events, EVENT_FLAG_OR, WAIT_FOREVER, &read_events);
    
    ARCH_INT_LOCK(intsave);
    checker->read_events = read_events;
    checker->test_end = 1;
    ARCH_INT_UNLOCK(intsave);
}

void_t helper_task_event_write_entry(void_t *data)
{
    if (data == NULL) {
        return;
    }
    st_event_checker *checker = (st_event_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->write_cnt = 1;
    ARCH_INT_UNLOCK(intsave);
    
    uint32_t res = osz_events_write(checker->events, checker->expected_events);
}
