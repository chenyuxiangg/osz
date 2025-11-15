/**
 * @file test_st_sem_g4_helper.c
 * @brief Helper functions implementation for semaphore timeout and scheduling interaction tests
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 */

#include "test_st_sem_g4_helper.h"
#include "inner_task_err.h"
#include "sem.h"
#include "task.h"
#include "et.h"

// uint32_t helper_task_create(uint8_t *task_name, uint32_t prio, uint32_t stack_size, task_callback_t func, void_t *data)
// {
//     uint16_t task_id = 0;
//     void_t *stack_addr = osz_malloc(0x800);
//     osz_task_params_t params = {
//         .name = "Test_Int_Release",
//         .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
//         .stack_base = (uintptr_t)stack_addr,
//         .stack_attr = STACK_MEM_DYNAMIC,
//         .stack_size = 0x800,
//         .thread = func,
//         .data = data,
//     };
//     uint32_t res = osz_create_task(&task_id, &params);
//     if (res != OS_OK) {
//         return res;
//     }
//     osz_task_resume(task_id);
//     return OS_OK;
// }

void_t helper_4_1_task_entry(void_t *data)
{
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    checker->pend_cnt++;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_pend(checker->sem, 0x200);
    printf("%s, %d\n", __FUNCTION__, __LINE__);
    ARCH_INT_LOCK(intsave);
    checker->post_cnt++;
    checker->test_end = 1;
    ARCH_INT_UNLOCK(intsave);
}