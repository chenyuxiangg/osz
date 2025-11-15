#include "test_st_sem_g3_helper.h"
#include "clint.h"
#include "mem.h"
#include "interrupt.h"
#include "platform.h"

#define CLINT_SOFTWARE_INT          (0x3)
#define SOFTWARE_INT_DEFAULT_PRI    (0x7)

osz_sem_t *g_int_test_sem = NULL;

void_t helper_software_int_handler_3_1(void_t *args)
{
    osz_sem_post(g_int_test_sem);
    CLINT_CLEAN_MSIP(CLINT_BASE_ADDR, PLIC_HART0_MACHINE);
}

void_t helper_software_int_handler_3_2(void_t *args)
{
    osz_sem_post(g_int_test_sem);
    osz_sem_post(g_int_test_sem);
    CLINT_CLEAN_MSIP(CLINT_BASE_ADDR, PLIC_HART0_MACHINE);
}

void_t helper_software_interrupt_init(interrupt_callback func)
{
    osz_interrupt_regist(CLINT_SOFTWARE_INT, func);
    osz_interrupt_enable(CLINT_SOFTWARE_INT);
    osz_interrupt_set_pri(CLINT_SOFTWARE_INT, SOFTWARE_INT_DEFAULT_PRI);
}

void_t helper_software_interrupt_deinit(void_t)
{
    osz_interrupt_unregist(CLINT_SOFTWARE_INT);
    osz_interrupt_disable(CLINT_SOFTWARE_INT);
    osz_interrupt_set_pri(CLINT_SOFTWARE_INT, 0);
}

void_t helper_software_interrupt_trigger(void_t)
{
    CLINT_PEND_MSIP(CLINT_BASE_ADDR, PLIC_HART0_MACHINE);
}

void_t helper_3_1_task_entry(void_t *data)
{
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    if (g_int_test_sem == NULL) {
        g_int_test_sem = checker->sem;
    }
    checker->pend_cnt++;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_pend(checker->sem, WAIT_FOREVER);
    ARCH_INT_LOCK(intsave);
    checker->post_cnt++;
    checker->test_end = 1;
    g_int_test_sem = NULL;
    ARCH_INT_UNLOCK(intsave);
}

void_t helper_3_2_task_entry(void_t *data)
{
    st_sem_checker *checker = (st_sem_checker *)data;
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    if (g_int_test_sem == NULL) {
        g_int_test_sem = checker->sem;
    }
    checker->pend_cnt++;
    ARCH_INT_UNLOCK(intsave);
    osz_sem_pend(checker->sem, WAIT_FOREVER);
    ARCH_INT_LOCK(intsave);
    checker->post_cnt++;
    ARCH_INT_UNLOCK(intsave);
}

uint32_t helper_task_create(uint8_t *task_name, uint32_t prio, uint32_t stack_size, task_callback_t func, void_t *data)
{
    uint16_t task_id = 0;
    void_t *stack_addr = osz_malloc(0x800);
    osz_task_params_t params = {
        .name = "Test_Int_Release",
        .priority = PRIORITY_FOR_TEST_SEM_DEFAULT,
        .stack_base = (uintptr_t)stack_addr,
        .stack_attr = STACK_MEM_DYNAMIC,
        .stack_size = 0x800,
        .thread = func,
        .data = data,
    };
    uint32_t res = osz_create_task(&task_id, &params);
    if (res != OS_OK) {
        return res;
    }
    osz_task_resume(task_id);
    return OS_OK;
}