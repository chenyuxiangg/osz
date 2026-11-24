#include "osz_test.h"

/* ============================================================================
 * 报告：稳定的行协议（v1）。宿主侧只认 [SZTEST] 前缀的行，其余输出原样透传。
 *
 *   [SZTEST] v1 suite_start total=N board=xxx
 *   [SZTEST] case_begin <suite>.<name>
 *   [SZTEST] assert_fail <file>:<line> "<expr>"
 *   [SZTEST] assert_detail <file>:<line> expect=0x.. actual=0x..
 *   [SZTEST] case_skip <file>:<line> "<reason>"
 *   [SZTEST] case_end <suite>.<name> PASS|FAIL|CRASH|SKIP ms=N asserts=N
 *   [SZTEST] suite_end pass=N fail=N crash=N skip=N ms=N
 * ========================================================================= */

void tc_report_suite_start(uint32_t total, const char *board)
{
    printf("[SZTEST] %s suite_start total=%u board=%s\n",
           TC_PROTOCOL_VERSION, total, (board != NULL) ? board : "unknown");
}

void tc_report_case_begin(const osz_test_case_t *c)
{
    if (c == NULL) {
        return;
    }
    printf("[SZTEST] case_begin %s.%s\n", c->suite, c->name);
}

void tc_report_assert(const char *file, uint32_t line, const char *msg)
{
    printf("[SZTEST] assert_fail %s:%u \"%s\"\n",
           (file != NULL) ? file : "?", line, (msg != NULL) ? msg : "?");
}

void tc_report_case_end(const osz_test_case_t *c, const char *status,
                        uint32_t asserts, uint32_t ms)
{
    if (c == NULL) {
        return;
    }
    printf("[SZTEST] case_end %s.%s %s ms=%u asserts=%u\n",
           c->suite, c->name, (status != NULL) ? status : "UNKNOWN", ms, asserts);
}

void tc_report_suite_end(uint32_t pass, uint32_t fail, uint32_t crash,
                         uint32_t skip, uint32_t ms)
{
    printf("[SZTEST] suite_end pass=%u fail=%u crash=%u skip=%u ms=%u\n",
           pass, fail, crash, skip, ms);
}
