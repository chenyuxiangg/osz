#include "osz_test.h"
#include "shell.h"
#include "string.h"

/* ============================================================================
 * shell 域：静态命令注册表（.osz_cmd 段）是否被正确链接与遍历
 *
 * 说明：这条用例自己用 REGISTER_STATIC_CMD 放一条探针命令进 .osz_cmd，
 * 从而在不依赖「某个模块恰好被链接进来」的前提下，验证
 *   ①链接脚本里 KEEP(*(.osz_cmd)) 没被 --gc-sections 丢掉
 *   ②section 边界符号 __cmd_start/__cmd_end 与 CMD_PARAMS 步长一致
 *   ③内核目录遍历能按名字命中
 * ========================================================================= */

extern uint8_t __cmd_start[];
extern uint8_t __cmd_end[];

STATIC void tc_probe_cmd_handler(uint32_t argc, int8_t *argv[])
{
    (void)argc;
    (void)argv;
}

REGISTER_STATIC_CMD(tc_selftest_probe, (CMD_CALLBACK_FUNC)tc_probe_cmd_handler, 1)

STATIC uint32_t tc_cmd_count(void)
{
    uintptr_t span = (uintptr_t)__cmd_end - (uintptr_t)__cmd_start;
    return (uint32_t)(span / sizeof(CMD_PARAMS));
}

STATIC uint32_t tc_cmd_find(const char *name)
{
    uint32_t found = 0;
    CMD_PARAMS *cmds = (CMD_PARAMS *)(void *)__cmd_start;
    uint32_t n = tc_cmd_count();

    for (uint32_t i = 0; i < n; ++i) {
        if ((cmds[i].cmd_name != NULL) &&
            (strncmp((VOID *)cmds[i].cmd_name, (VOID *)name,
                     OSZ_CFG_OBJ_NAME_MAX_LENS) == 0)) {
            found++;
        }
    }
    return found;
}

OSZ_TEST(shell, cmd_table, .tags = TC_TAG_FAST, .cost = 1)
{
    uint32_t n = tc_cmd_count();

    /* 段里至少有本用例放进去的那条探针命令 */
    TC_ASSERT(n >= 1);

    /* 段跨度必须是 CMD_PARAMS 的整数倍：链接脚本与结构体步长一致 */
    TC_EXPECT_EQ(((uintptr_t)__cmd_end - (uintptr_t)__cmd_start) % sizeof(CMD_PARAMS), 0u);

    /* 遍历能按名字命中探针命令，且只命中一次 */
    TC_EXPECT_EQ(tc_cmd_find("tc_selftest_probe"), 1u);
}
