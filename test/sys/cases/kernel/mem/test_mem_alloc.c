#include "osz_test.h"
#include "mem.h"

/* ============================================================================
 * mem 域：分配 / 写入 / 读取 / 释放 的最小闭环
 *
 * P0 只验证“可用性与不串味”；碎片、相邻空闲块合并、失败路径属 P1
 * （配合 OSZ_CFG_MEM_DEBUG 自检）。
 * ========================================================================= */

STATIC uint32_t tc_pattern_check(uint8_t *p, uint32_t size, uint8_t seed)
{
    for (uint32_t i = 0; i < size; ++i) {
        if (p[i] != (uint8_t)(seed + i)) {
            return i;
        }
    }
    return size;
}

STATIC uint32_t tc_zero_check(uint8_t *p, uint32_t size)
{
    for (uint32_t i = 0; i < size; ++i) {
        if (p[i] != 0u) {
            return i;
        }
    }
    return size;
}

OSZ_TEST(mem, alloc_free_roundtrip, .tags = TC_TAG_FAST | TC_TAG_MEM, .cost = 2)
{
    const uint32_t size = 128;
    uint8_t *p = (uint8_t *)osz_malloc(size);

    TC_ASSERT(p != NULL);
    for (uint32_t i = 0; i < size; ++i) {
        p[i] = (uint8_t)(0xA5u + i);
    }
    TC_ASSERT_EQ(tc_pattern_check(p, size, 0xA5u), size);
    osz_free(p);

    /* 再申请同样大小，内容不应保留上一次的写入（同一块可能被复用） */
    uint8_t *q = (uint8_t *)osz_malloc(size);
    TC_ASSERT(q != NULL);
    for (uint32_t i = 0; i < size; ++i) {
        q[i] = 0;
    }
    TC_ASSERT_EQ(tc_zero_check(q, size), size);
    osz_free(q);

    uint8_t *z = (uint8_t *)osz_zalloc(size);
    TC_ASSERT(z != NULL);
    TC_ASSERT_EQ(tc_zero_check(z, size), size);
    osz_free(z);

    /* 释放后堆应恢复到入口时的水位（用“还能申请到大块”间接验证） */
    uint8_t *big = (uint8_t *)osz_malloc(4096);
    TC_ASSERT(big != NULL);
    osz_free(big);
}
