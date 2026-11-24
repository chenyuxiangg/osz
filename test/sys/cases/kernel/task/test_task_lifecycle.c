#include "osz_test.h"
#include "task.h"
#include "mem.h"
#include "inner_task_err.h"

/* ============================================================================
 * task 域：参数校验、创建/删除、当前任务身份
 *
 * 约束：用例任务自己不能被删除，因此只用“创建后立即删除”的方式验证
 * 生命周期；所有 stack 都用 osz_malloc 申请并交给 delete 释放。
 * ========================================================================= */

STATIC void tc_task_sleep_forever(void_t *arg)
{
    (void)arg;
    while (1) {
        osz_msleep(1000);
    }
}

STATIC uint32_t tc_free_task_count(void)
{
    uint32_t free_cnt = 0;

    for (uint32_t i = 0; i < OSZ_CFG_TASK_LIMIT; ++i) {
        if ((TASK_STATE(i) & TSK_FLAG_FREE) != 0) {
            free_cnt++;
        }
    }
    return free_cnt;
}

OSZ_TEST(task, create_bad_params, .tags = TC_TAG_FAST, .cost = 2)
{
    static int8_t stack_buf[512] __attribute__((aligned(16)));
    uint16_t tid = 0xFFFF;
    uint32_t ret = 0;

    osz_task_params_t params = {
        .name = (int8_t *)"tcbad",
        .stack_attr = STACK_MEM_STATIC,
        .stack_base = (uintptr_t)stack_buf,
        .stack_size = sizeof(stack_buf),
        .priority = 10,
        .thread = (task_callback_t)tc_task_sleep_forever,
        .data = NULL,
    };

    params.name = NULL;
    ret = osz_create_task(&tid, &params);
    TC_EXPECT_EQ(ret, TASK_CREATE_CHECK_NAME_ERR);

    params.name = (int8_t *)"tcbad";
    params.stack_base = (uintptr_t)NULL;
    ret = osz_create_task(&tid, &params);
    TC_EXPECT_EQ(ret, TASK_CREATE_CHECK_STACK_BASE_ERR);

    params.stack_base = (uintptr_t)stack_buf;
    params.stack_size = 0;
    ret = osz_create_task(&tid, &params);
    TC_EXPECT_EQ(ret, TASK_CREATE_CHECK_STACK_SIZE_ERR);

    params.stack_size = sizeof(stack_buf);
    params.thread = NULL;
    ret = osz_create_task(&tid, &params);
    TC_EXPECT_EQ(ret, TASK_CREATE_CHECK_TASK_ENTRY_ERR);

    /* 以上四次都应在参数校验阶段被拒，不得消耗 task 槽位 */
    TC_ASSERT(tc_free_task_count() > 0u);
}

OSZ_TEST(task, create_delete_roundtrip, .tags = TC_TAG_FAST, .cost = 2)
{
    uint32_t free_before = tc_free_task_count();
    uint16_t tid = 0xFFFF;
    void_t *stack = osz_malloc(0x400);

    TC_ASSERT(stack != NULL);

    osz_task_params_t params = {
        .name = (int8_t *)"tctmp",
        .stack_attr = STACK_MEM_DYNAMIC,
        .stack_base = (uintptr_t)stack,
        .stack_size = 0x400,
        .priority = 12,
        .thread = (task_callback_t)tc_task_sleep_forever,
        .data = NULL,
    };

    TC_ASSERT_EQ(osz_create_task(&tid, &params), (uintptr_t)OS_OK);
    TC_ASSERT(tid < OSZ_CFG_TASK_LIMIT);
    TC_ASSERT((TASK_STATE(tid) & TSK_FLAG_FREE) == 0);
    TC_ASSERT_EQ(tc_free_task_count() + 1u, free_before);

    /* delete 会回收动态栈，之后不得再 free 同一块 */
    osz_delete_task(tid);
    TC_ASSERT((TASK_STATE(tid) & TSK_FLAG_FREE) != 0);
    TC_ASSERT_EQ(tc_free_task_count(), free_before);
}

OSZ_TEST(task, current_identity, .tags = TC_TAG_FAST, .cost = 1)
{
    uint32_t tid = osz_get_current_tid();

    TC_ASSERT(tid < OSZ_CFG_TASK_LIMIT);
    TC_ASSERT((TASK_STATE(tid) & TSK_FLAG_FREE) == 0);
    TC_ASSERT((TASK_STATE(tid) & TSK_FLAG_RUNNING) != 0);
    TC_ASSERT_EQ(osz_get_task_priotity_by_tid(tid), TASK_PRIORITY(tid));
}
