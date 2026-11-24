#include "osz_test.h"
#include "shell.h"
#include "string.h"

/* ============================================================================
 * 板级手动入口：产品形态镜像里的 `test` 命令
 *
 *   OSZ$ test                    列出全部用例（名字 + 标签 + 预算）
 *   OSZ$ test task.current_identity   只跑这一条，输出与 autorun 同一套行协议
 *
 * 存在的意义：现场定位时不用插拔镜像、不用重编，直接在产品形态镜像里
 * 单跑一条用例，拿到 FAIL/CRASH 的证据。
 * ========================================================================= */

#define TC_CMD_MAX_ARG  4

STATIC void tc_cmd_list(void)
{
    uint32_t total = tc_case_total();

    printf("[SZTEST] list total=%u\n", total);
    for (uint32_t i = 0; i < total; ++i) {
        const osz_test_case_t *c = tc_case_at(i);
        if (c == NULL) {
            continue;
        }
        printf("  %s.%s tags=%#x cost=%u\n", c->suite, c->name, c->tags, c->cost);
    }
}

STATIC void tc_cmd_handler(uint32_t argc, int8_t *argv[])
{
    if (argc <= 1) {
        tc_cmd_list();
        return;
    }
    if (strncmp((const char *)argv[1], "list", 8) == 0) {
        tc_cmd_list();
        return;
    }
    (void)tc_run_one((const char *)argv[1]);
}

REGISTER_STATIC_CMD(test, (CMD_CALLBACK_FUNC)tc_cmd_handler, TC_CMD_MAX_ARG)
