#include "osz_test.h"
#include "task.h"
#include "timer.h"
#include "string.h"

#define TC_TIME_JUMP_LIMIT_MS   (60000u)   /* 单次读数允许的最大前进量 */

/* ============================================================================
 * runner：注册表遍历 + 用例隔离 + 状态归零 + 报告串联
 *
 * 上下文：本函数由测试镜像的 app_main 调用，也就是说运行在 kernel 为
 * app_main 创建的静态栈任务里（OSZ_CFG_TASK_STACK_DEFAULT_SIZE）。
 * ========================================================================= */

STATIC tc_jmp_buf_t             g_landing;
STATIC const osz_test_case_t   *g_active;
STATIC volatile uint32_t        g_assert_total;
STATIC volatile uint32_t        g_assert_failed;
STATIC volatile uint8_t         g_crashed;
STATIC volatile uint8_t         g_skipped;

/* ------------------------------ 时间 ------------------------------------- */
uint32_t tc_now_ms(void)
{
    UINT64 us = 0;
    UINT32 now_ms = 0;

    /* 单调钳位：某些仿真后端对 mtime 的 64 位读会偶发返回异常值，
     * 一旦出现「时间倒流」或「跳跃过大」就沿用上次值，避免报告里出现
     * 荒谬的用例耗时（平台缺陷另行跟踪，不在 harness 里硬编码修补）。 */
    static UINT32 last_ms = 0;

    if (drv_timer_get_time_stamp(&us) != OS_OK) {
        return last_ms;
    }
    now_ms = (UINT32)(us / 1000u);
    if ((now_ms < last_ms) || ((now_ms - last_ms) > TC_TIME_JUMP_LIMIT_MS)) {
        return last_ms;
    }
    last_ms = now_ms;
    return now_ms;
}

void tc_sleep_ms(uint32_t ms)
{
    osz_msleep((uint64_t)ms);
}

/* ------------------------------ 注册表 ----------------------------------- */
uint32_t tc_case_total(void)
{
    uintptr_t span = (uintptr_t)__test_end - (uintptr_t)__test_start;
    return (uint32_t)(span / sizeof(osz_test_case_t));
}

const osz_test_case_t *tc_case_at(uint32_t idx)
{
    if (idx >= tc_case_total()) {
        return NULL;
    }
    return (const osz_test_case_t *)((uintptr_t)__test_start +
                                     idx * sizeof(osz_test_case_t));
}

/* --------------------------- 用例状态记账 -------------------------------- */
void tc_impl_note_assert(uint32_t failed)
{
    g_assert_total++;
    if (failed) {
        g_assert_failed++;
    }
}

void tc_impl_mark_crash(void)
{
    g_crashed = 1;
}

void tc_impl_mark_skip(void)
{
    g_skipped = 1;
}

void tc_impl_recover(void)
{
    tc_longjmp(&g_landing);
}

/*
 * 同步异常（野指针 / 非法指令 / 对齐错）落这里。
 * 只有测试任务自己在跑用例时才可能进入，因此 sp 仍是该任务栈，
 * longjmp 是安全的；异步中断不会走到这个入口。
 */
void tc_exception_recover(uint32_t mcause, uint32_t mepc, uint32_t mtval)
{
    if (g_active == NULL) {
        printf("[SZTEST] fatal outside case mcause=%#x mepc=%#x mtval=%#x\n",
               mcause, mepc, mtval);
        while (1) {
        }
    }
    tc_impl_mark_crash();
    printf("[SZTEST] crash_detail mcause=%#x mepc=%#x mtval=%#x\n",
           mcause, mepc, mtval);
    tc_longjmp(&g_landing);
}

/* ------------------------------ 单条用例 --------------------------------- */
/*
 * 执行一条用例并返回状态字符串。
 * 全量与手动单跑共用同一段逻辑，保证「一条用例怎么跑」只有一份定义。
 */
STATIC const char *inner_run_case(const osz_test_case_t *c, uint32_t *out_ms)
{
    const char  *status = "PASS";
    uint32_t     c0 = 0;
    uint32_t     dt = 0;

    g_active = c;
    g_assert_total = 0;
    g_assert_failed = 0;
    g_crashed = 0;
    g_skipped = 0;

    tc_report_case_begin(c);
    c0 = tc_now_ms();

    if (tc_setjmp(&g_landing) == 0) {
        c->fn();
    }

    dt = tc_now_ms() - c0;

    if (g_crashed) {
        status = "CRASH";
    } else if (g_skipped) {
        status = "SKIP";
    } else if (g_assert_failed != 0) {
        status = "FAIL";
    }

    tc_report_case_end(c, status, g_assert_total, dt);
    g_active = NULL;

    if (out_ms != NULL) {
        *out_ms = dt;
    }
    return status;
}

/* ------------------------- 名字匹配（"suite.name"） ----------------------- */
/* "suite.name" 缓冲：按用例命名的实际上限给（配置里的对象名长度不够用） */
#define TC_DOTTED_MAX   (64)

STATIC void inner_build_dotted(const osz_test_case_t *c, char *buf, uint32_t len)
{
    uint32_t i = 0;
    uint32_t j = 0;

    while ((c->suite[i] != '\0') && (i + 1 < len)) {
        buf[i] = c->suite[i];
        i++;
    }
    if (i + 1 < len) {
        buf[i++] = '.';
    }
    while ((c->name[j] != '\0') && (i + 1 < len)) {
        buf[i++] = c->name[j];
        j++;
    }
    buf[i] = '\0';
}

const osz_test_case_t *tc_case_find(const char *dotted)
{
    uint32_t total = tc_case_total();
    char buf[TC_DOTTED_MAX];

    if (dotted == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < total; ++i) {
        const osz_test_case_t *c = tc_case_at(i);
        if ((c == NULL) || (c->fn == NULL)) {
            continue;
        }
        inner_build_dotted(c, buf, sizeof(buf));
        if (strncmp(buf, (VOID *)dotted, TC_DOTTED_MAX) == 0) {
            return c;
        }
    }
    return NULL;
}

/* ------------------------------ 主循环 ----------------------------------- */
int tc_run_all(const char *board)
{
    uint32_t total = tc_case_total();
    uint32_t pass = 0;
    uint32_t fail = 0;
    uint32_t crash = 0;
    uint32_t skip = 0;
    uint32_t t0 = tc_now_ms();

    tc_report_suite_start(total, board);

    for (uint32_t i = 0; i < total; ++i) {
        const osz_test_case_t *c = tc_case_at(i);
        const char *status = NULL;

        if ((c == NULL) || (c->fn == NULL)) {
            continue;
        }

        status = inner_run_case(c, NULL);
        if (status[0] == 'C') {              /* CRASH */
            crash++;
        } else if (status[0] == 'S') {       /* SKIP  */
            skip++;
        } else if (status[0] == 'F') {       /* FAIL  */
            fail++;
        } else {                             /* PASS  */
            pass++;
        }
    }

    tc_report_suite_end(pass, fail, crash, skip, tc_now_ms() - t0);
    printf("[SZTEST] end fails=%u\n", fail + crash);
    return (int)(fail + crash);
}

/*
 * 手动单跑：产品形态镜像里 `test <suite.name>` 走这里。
 * 只跑命中的一条，但仍然走完整的报告协议，供宿主 runner 复用同一套解析。
 */
int tc_run_one(const char *case_name)
{
    const osz_test_case_t *c = tc_case_find(case_name);
    const char *status = NULL;
    uint32_t total = tc_case_total();

    tc_report_suite_start(total, "manual");
    if (c == NULL) {
        printf("[SZTEST] case_not_found %s\n", case_name);
        tc_report_suite_end(0, 1, 0, 0, 0);
        printf("[SZTEST] end fails=1\n");
        return 1;
    }

    status = inner_run_case(c, NULL);
    tc_report_suite_end(status[0] == 'P' ? 1 : 0,
                        status[0] == 'F' ? 1 : 0,
                        status[0] == 'C' ? 1 : 0,
                        status[0] == 'S' ? 1 : 0, 0);
    printf("[SZTEST] end fails=%d\n",
           (int)((status[0] == 'F') || (status[0] == 'C')));
    return (int)((status[0] == 'F') || (status[0] == 'C'));
}
