#ifndef __OSZ_TEST_H__
#define __OSZ_TEST_H__

#include "comm.h"

/* ============================================================================
 * OSZ on-target system test harness —— 用例宏 / 断言 / 恢复 / 报告
 *
 * 设计要点：
 *   1. 用例元数据是编译期常量，放在 .osz_test 段，链接期由 osz.ld 的
 *      __test_start / __test_end 圈住（KEEP 保证不被 --gc-sections 收走）。
 *   2. 只用公共 API 与 printf，不依赖 stdio / malloc / C++。
 *   3. 致命失败与同步异常都通过 longjmp 回到 runner，用例函数不再继续执行。
 * ========================================================================= */

/* ---------------------------- 元数据位图 ---------------------------------- */
#define TC_TAG_FAST         (1u << 0)
#define TC_TAG_SLOW         (1u << 1)
#define TC_TAG_IRQ          (1u << 2)
#define TC_TAG_MEM          (1u << 3)
#define TC_TAG_NONDET       (1u << 4)

#define TC_CAP_UART         (1u << 0)
#define TC_CAP_TIMER        (1u << 1)
#define TC_CAP_PLIC         (1u << 2)
#define TC_CAP_SPI          (1u << 3)
#define TC_CAP_I2C          (1u << 4)
#define TC_CAP_DMA          (1u << 5)

#define TC_PROTOCOL_VERSION "v1"

typedef enum {
    TC_KIND_FUNC = 0,
    TC_KIND_PERF,
    TC_KIND_SOAK,
    TC_KIND_CONTRACT,
} tc_kind_t;

typedef struct {
    const char *suite;
    const char *name;
    void      (*fn)(void);
    uint32_t    tags;
    uint32_t    caps;
    uint16_t    timeout_ms;
    uint8_t     cost;
    uint8_t     kind;
    uint16_t    version;
} osz_test_case_t;

/* --------------------------- 注册（链接段） ------------------------------- */
#define TC_CASE_SECTION  __attribute__((section(".osz_test"), used, aligned(4)))

/*
 * OSZ_TEST(suite, name, ...) —— 每个用例定义一条元数据 + 一个函数。
 * 只建议在独立 .c 文件中使用（同一文件内多个用例会重名冲突）。
 */
#define OSZ_TEST(suite_, name_, ...)                                  \
    static void tc_case_fn_##name_(void);                             \
    static const osz_test_case_t tc_case_meta_##name_                 \
        TC_CASE_SECTION = {                                           \
            .suite  = #suite_,                                        \
            .name   = #name_,                                         \
            .fn     = tc_case_fn_##name_,                             \
            .kind   = TC_KIND_FUNC,                                   \
            .cost   = 2,                                              \
            .version = 1,                                             \
            __VA_ARGS__ };                                            \
    static void tc_case_fn_##name_(void)

extern osz_test_case_t __test_start[];
extern osz_test_case_t __test_end[];

/* ------------------------------ 断言 ------------------------------------- */
/* 断言入口由 assert.c 实现，宏在下面展开时引用 */
void tc_assert_fail(const char *file, uint32_t line, const char *expr, uint32_t fatal);
void tc_assert_eq_fail(const char *file, uint32_t line,
                       uintptr_t expect, uintptr_t actual, uint32_t fatal);
void tc_skip(const char *file, uint32_t line, const char *reason);

#define TC_ASSERT(cond) do {                                                  \
        if (!(cond)) {                                                        \
            tc_assert_fail(__FILE__, __LINE__, #cond, 1);                     \
        } else {                                                              \
            tc_impl_note_assert(0);                                           \
        }                                                                     \
    } while (0)

#define TC_EXPECT(cond) do {                                                  \
        if (!(cond)) {                                                        \
            tc_assert_fail(__FILE__, __LINE__, #cond, 0);                     \
        } else {                                                              \
            tc_impl_note_assert(0);                                           \
        }                                                                     \
    } while (0)

#define TC_ASSERT_EQ(expect, actual) do {                                     \
        uintptr_t tc_e_ = (uintptr_t)(expect);                                \
        uintptr_t tc_a_ = (uintptr_t)(actual);                                \
        if (tc_e_ != tc_a_) {                                                 \
            tc_assert_eq_fail(__FILE__, __LINE__, tc_e_, tc_a_, 1);           \
        } else {                                                              \
            tc_impl_note_assert(0);                                           \
        }                                                                     \
    } while (0)

#define TC_EXPECT_EQ(expect, actual) do {                                     \
        uintptr_t tc_e_ = (uintptr_t)(expect);                                \
        uintptr_t tc_a_ = (uintptr_t)(actual);                                \
        if (tc_e_ != tc_a_) {                                                 \
            tc_assert_eq_fail(__FILE__, __LINE__, tc_e_, tc_a_, 0);           \
        } else {                                                              \
            tc_impl_note_assert(0);                                           \
        }                                                                     \
    } while (0)

#define TC_SKIP(reason) tc_skip(__FILE__, __LINE__, (reason))

/* ------------------------- runner 与恢复接口 ----------------------------- */
/* 14 个寄存器槽：ra / sp / s0-s11 */
typedef struct {
    uint32_t reg[14];
} tc_jmp_buf_t;

int  tc_setjmp(tc_jmp_buf_t *buf) __attribute__((returns_twice));
void tc_longjmp(tc_jmp_buf_t *buf) __attribute__((noreturn));

uint32_t tc_case_total(void);
const osz_test_case_t *tc_case_at(uint32_t idx);
const osz_test_case_t *tc_case_find(const char *dotted);   /* "suite.name" */
int  tc_run_all(const char *board);
int  tc_run_one(const char *case_name);                    /* 手动单跑一条 */

/* ------------------------- 内部：状态与报告 ------------------------------- */
void tc_impl_note_assert(uint32_t failed);
void tc_impl_mark_crash(void);
void tc_impl_mark_skip(void);
void tc_impl_recover(void) __attribute__((noreturn));

void tc_report_suite_start(uint32_t total, const char *board);
void tc_report_case_begin(const osz_test_case_t *c);
void tc_report_case_end(const osz_test_case_t *c, const char *status,
                        uint32_t asserts, uint32_t ms);
void tc_report_assert(const char *file, uint32_t line, const char *msg);
void tc_report_suite_end(uint32_t pass, uint32_t fail, uint32_t crash,
                         uint32_t skip, uint32_t ms);

/* ------------------------------ 便捷函数 --------------------------------- */
void     tc_sleep_ms(uint32_t ms);
uint32_t tc_now_ms(void);

#endif /* __OSZ_TEST_H__ */
