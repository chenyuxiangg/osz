#include "osz_test.h"
#include "event.h"

/* ============================================================================
 * ipc/event 域：OR + CLEAN 语义
 *
 * 只做不依赖其他任务的同步验证：
 *   write(bit) → read(mask, OR|CLEAN) 应立刻拿到该位，且 CLEAN 后清掉。
 * ========================================================================= */

OSZ_TEST(ipc, event_or_clean, .tags = TC_TAG_FAST, .cost = 2)
{
    osz_events_t *ecb = NULL;
    uint32_t out = 0;
    uint32_t ret = 0;

    ret = osz_events_init(NULL, 0, &ecb);
    TC_ASSERT_EQ(ret, (uintptr_t)OS_OK);
    TC_ASSERT(ecb != NULL);

    ret = osz_events_write(ecb, 0x1);
    TC_ASSERT_EQ(ret, (uintptr_t)OS_OK);

    ret = osz_events_read(ecb, 0x1, (osz_events_op_t)(EVENT_FLAG_OR | EVENT_FLAG_CLEAN),
                          0, &out);
    TC_ASSERT_EQ(ret, (uintptr_t)OS_OK);
    TC_EXPECT_EQ(out, 1u);

    ret = osz_events_write(ecb, 0x2);
    TC_ASSERT_EQ(ret, (uintptr_t)OS_OK);

    ret = osz_events_read(ecb, 0x3, (osz_events_op_t)(EVENT_FLAG_OR | EVENT_FLAG_CLEAN),
                          0, &out);
    TC_ASSERT_EQ(ret, (uintptr_t)OS_OK);
    TC_EXPECT_EQ(out, 2u);

    /* CLEAN 之后同一位不应再被读到（不阻塞，用 1ms 兜底） */
    ret = osz_events_read(ecb, 0x1, (osz_events_op_t)(EVENT_FLAG_OR | EVENT_FLAG_CLEAN),
                          1, &out);
    TC_EXPECT(ret != OS_OK);

    (void)osz_events_delete(ecb);
}
