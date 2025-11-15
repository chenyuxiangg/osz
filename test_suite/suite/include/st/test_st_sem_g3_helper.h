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

/**
 * @brief Data structure for interrupt semaphore release test
 */
typedef struct {
    osz_sem_t *sem;          // Pointer to semaphore
    uint32_t *wake_flag;     // Pointer to wake flag
} st_sem_int_release_data_t;

/**
 * @brief Data structure for interrupt safety test
 */
typedef struct {
    osz_sem_t *sem;          // Pointer to semaphore
    uint32_t *op_count;      // Pointer to operation count
    uint32_t op_type;        // 0 for pend, 1 for post
} st_sem_int_safety_data_t;

/**
 * @brief Data structure for interrupt wait queue test
 */
typedef struct {
    osz_sem_t *sem;          // Pointer to semaphore
    uint32_t *wake_order;    // Pointer to wake order array
    uint32_t task_id;        // Task identifier
} st_sem_int_queue_data_t;

/**
 * @brief Data structure for nested interrupt test
 */
typedef struct {
    osz_sem_t *sem;          // Pointer to semaphore
    uint32_t *nest_level;    // Pointer to nesting level
} st_sem_nested_int_data_t;

/**
 * @brief Data structure for interrupt-task coordination test
 */
typedef struct {
    osz_sem_t *sem;          // Pointer to semaphore
    uint32_t *data_processed;// Pointer to data processed flag
    uint8_t *data_buffer;    // Pointer to data buffer
} st_sem_int_coord_data_t;

/**
 * @brief Task entry function for interrupt semaphore release test
 * @param arg Pointer to st_sem_int_release_data_t
 */
void_t helper_task_sem_int_release_entry(void_t *arg);

/**
 * @brief Task entry function for interrupt safety test
 * @param arg Pointer to st_sem_int_safety_data_t
 */
void_t helper_task_sem_int_safety_entry(void_t *arg);

/**
 * @brief Task entry function for interrupt wait queue test
 * @param arg Pointer to st_sem_int_queue_data_t
 */
void_t helper_task_sem_int_queue_entry(void_t *arg);

/**
 * @brief Task entry function for nested interrupt test
 * @param arg Pointer to st_sem_nested_int_data_t
 */
void_t helper_task_sem_nested_int_entry(void_t *arg);

/**
 * @brief Task entry function for interrupt-task coordination test
 * @param arg Pointer to st_sem_int_coord_data_t
 */
void_t helper_task_sem_int_coord_entry(void_t *arg);

/**
 * @brief Interrupt callback function for semaphore release test
 * @param args Pointer to semaphore
 */
void_t helper_int_sem_release_callback(void_t *args);

/**
 * @brief Interrupt callback function for multiple semaphore releases
 * @param args Pointer to semaphore
 */
void_t helper_int_sem_multiple_release_callback(void_t *args);

/**
 * @brief Interrupt callback function for nested interrupt test
 * @param args Pointer to st_sem_nested_int_data_t
 */
void_t helper_int_sem_nested_callback(void_t *args);

/**
 * @brief Interrupt callback function for data ready signal
 * @param args Pointer to st_sem_int_coord_data_t
 */
void_t helper_int_data_ready_callback(void_t *args);

#endif /* TEST_ST_SEM_G3_HELPER_H */
