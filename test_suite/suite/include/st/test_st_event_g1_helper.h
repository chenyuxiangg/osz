#ifndef TEST_ST_EVENT_G1_HELPER_H
#define TEST_ST_EVENT_G1_HELPER_H

#include "event.h"
#include "task.h"

typedef struct {
    osz_events_t *events;
    uint32_t read_cnt;
    uint32_t write_cnt;
    uint32_t test_end;
    uint32_t read_events;
    uint32_t expected_events;
} st_event_checker;

void_t helper_task_event_read_entry(void_t *data);
void_t helper_task_event_write_entry(void_t *data);

#endif
