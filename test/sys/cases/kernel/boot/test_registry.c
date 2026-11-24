#include "osz_test.h"

/* ============================================================================
 * boot 域：验证“测试镜像自己是否装对了”
 *   - 用例确实被链接进 .osz_test 段（防止 0 用例静默通过）
 *   - shell 静态命令表非空（守护 .osz_cmd 段与 KEEP 规则）
 * ========================================================================= */

OSZ_TEST(boot, registry_nonempty, .tags = TC_TAG_FAST, .cost = 1)
{
    uint32_t total = tc_case_total();

    TC_ASSERT(total > 0);

    for (uint32_t i = 0; i < total; ++i) {
        const osz_test_case_t *c = tc_case_at(i);
        TC_ASSERT(c != NULL);
        TC_ASSERT(c->fn != NULL);
        TC_ASSERT(c->suite != NULL);
        TC_ASSERT(c->name != NULL);
    }
}
