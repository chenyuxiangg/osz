#include "osz_test.h"

/* ============================================================================
 * 断言实现
 *
 * 非致命失败：只记账并返回，用例继续跑（一次跑出多个差异点）。
 * 致命失败：记账后 longjmp 回 runner，用例函数不再返回。
 * ========================================================================= */

void tc_assert_fail(const char *file, uint32_t line, const char *expr, uint32_t fatal)
{
    tc_impl_note_assert(1);
    tc_report_assert(file ? file : "?", line, expr ? expr : "?");
    if (fatal) {
        tc_impl_recover();
    }
}

void tc_assert_eq_fail(const char *file, uint32_t line,
                       uintptr_t expect, uintptr_t actual, uint32_t fatal)
{
    tc_impl_note_assert(1);
    printf("[SZTEST] assert_fail %s:%u \"value mismatch\"\n",
           file ? file : "?", line);
    printf("[SZTEST] assert_detail %s:%u expect=%#x actual=%#x\n",
           file ? file : "?", line, (uint32_t)expect, (uint32_t)actual);
    if (fatal) {
        tc_impl_recover();
    }
}

void tc_skip(const char *file, uint32_t line, const char *reason)
{
    tc_impl_mark_skip();
    printf("[SZTEST] case_skip %s:%u \"%s\"\n",
           file ? file : "?", line, reason ? reason : "");
    tc_impl_recover();
}
