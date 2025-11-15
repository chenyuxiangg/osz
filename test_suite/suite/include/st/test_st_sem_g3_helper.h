#ifndef TEST_ST_SEM_G3_HELPER_H
#define TEST_ST_SEM_G3_HELPER_H

#include "comm.h"
#include "sem.h"
#include "task.h"
#include "interrupt.h"

typedef struct {
    osz_sem_t *sem;
    uint32_t pend_cnt : 8;
    uint32_t post_cnt : 8;
    uint32_t test_end : 1;
    uint32_t wake_task : 3;
    uint32_t reserv : 12;
} st_sem_checker;

typedef struct {
    st_sem_checker *checker;
    uint32_t data;
} st_sem_task_input;

#define PRIORITY_FOR_TEST_SEM_DEFAULT   (0x9)

void_t helper_software_interrupt_init(interrupt_callback func);
void_t helper_software_interrupt_trigger(void_t);
void_t helper_software_interrupt_deinit(void_t);
uint32_t helper_task_create(uint8_t *task_name, uint32_t prio, uint32_t stack_size, task_callback_t func, void_t *data);
void_t helper_3_1_task_entry(void_t *data);
void_t helper_3_2_task_entry(void_t *data);
void_t helper_software_int_handler_3_1(void_t *args);
void_t helper_software_int_handler_3_2(void_t *args);

#endif /* TEST_ST_SEM_G3_HELPER_H */
