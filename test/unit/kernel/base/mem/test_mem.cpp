// Unit tests for OSZ's best-fit heap with free-block coalescing.
//
// Unit convention: every block records its *span* = block header + payload,
// i.e. the byte distance from this block's header to the next block's header.
// Requires/requests are expressed in payload bytes and converted exactly once
// (inner_mem_alloc adds MEM_NODE_INFO_SIZE); split / merge / list ordering /
// self-check all work on spans.
//
// Public entry points under test (kernel/base/mem/bestfit/src/bestfit.c):
//   inner_mem_init(pool, size)   - injectable heap region
//   inner_mem_alloc(size, &out)  - best-fit + low-side split + rem merge
//   inner_mem_free(ptr)          - backward then forward coalescing
//
// The same test source is registered twice by CMakeLists.txt:
//   test_mem        -> OSZ_CFG_MEM_DEBUG undefined (0)
//   test_mem_debug  -> OSZ_CFG_MEM_DEBUG=1
// Both must show identical external behaviour; DEBUG only adds self-checks.
//
// Heap layout (16 KiB injected buffer):
//   [pool 16][head 16][first node 16][ ... payload ... ][end node 16]
// First block span = 16384 - pool - head - end_header; payload = span - header.

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <random>
#include <vector>

extern "C" {
#include "bestfit.h"
}

namespace {

constexpr std::uint32_t kHeapSize = 0x4000;                     // 16 KiB
constexpr std::uint32_t kStructOverhead = 3 * MEM_NODE_INFO_SIZE; // pool + head + end

alignas(64) std::uint8_t g_heap[kHeapSize];

// 首块占位（span），包含首块头
constexpr std::uint32_t FirstSpan() { return kHeapSize - kStructOverhead; }
// 单个申请可用的最大数据区
constexpr std::uint32_t MaxPayload() { return FirstSpan() - MEM_NODE_INFO_SIZE; }
// 一个申请实际占用的 span
constexpr std::uint32_t SpanOf(std::uint32_t payload) {
    return MEM_NODE_INFO_SIZE + ((payload + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1u));
}

MEM_NODE_INFO *EndNode() {
    return reinterpret_cast<MEM_NODE_INFO *>(inner_mem_heap_end() - MEM_NODE_INFO_SIZE);
}

MEM_NODE_INFO *FirstNode() { return inner_mem_head()->first_node; }

MEM_NODE_INFO *NodeOf(void *p) {
    return reinterpret_cast<MEM_NODE_INFO *>(static_cast<std::uint8_t *>(p) -
                                             MEM_NODE_INFO_SIZE);
}

// Physical walk of the block chain: first_node .. end_node.
std::vector<MEM_NODE_INFO *> WalkBlocks() {
    std::vector<MEM_NODE_INFO *> out;
    MEM_NODE_INFO *end = EndNode();
    MEM_NODE_INFO *cur = FirstNode();
    while (cur != end && out.size() < 4096) {
        out.push_back(cur);
        cur = MEM_NODE_NEXT(cur);
    }
    return out;
}

// Structural invariants: contiguity, alignment, span granularity, min block,
// prev-span consistency, I7 (no two adjacent free blocks), end node shape.
void ExpectChainSane() {
    MEM_NODE_INFO *end = EndNode();
    MEM_NODE_INFO *cur = FirstNode();
    MEM_NODE_INFO *prev = nullptr;
    std::size_t guard = 0;

    while (cur != end) {
        ASSERT_LT(guard++, 4096u);
        ASSERT_EQ(reinterpret_cast<std::uintptr_t>(cur) % MEM_ALIGN, 0u);
        ASSERT_EQ(MEM_NODE_GET_SPAN(cur) % MEM_ALIGN, 0u);
        ASSERT_GE(MEM_NODE_GET_SPAN(cur), MEM_MIN_BLOCK) << "block below min block";
        ASSERT_EQ(cur->prev_size & MEM_PREV_MAGIC_MASK, MEM_PREV_MAGIC);
        if (prev == nullptr) {
            ASSERT_EQ(MEM_NODE_GET_PREV_SPAN(cur), 0u);
        } else {
            ASSERT_EQ(MEM_NODE_NEXT(prev), cur);
            ASSERT_EQ(MEM_NODE_GET_PREV_SPAN(cur), MEM_NODE_GET_SPAN(prev));
        }
        if (MEM_NODE_GET_USED(cur) == 0) {
            MEM_NODE_INFO *next = MEM_NODE_NEXT(cur);
            if (next != end) {
                ASSERT_NE(MEM_NODE_GET_USED(next), 0u) << "adjacent free blocks";
            }
        }
        prev = cur;
        cur = MEM_NODE_NEXT(cur);
    }
    ASSERT_EQ(cur, end);
    ASSERT_NE(MEM_NODE_GET_USED(end), 0u);
    ASSERT_EQ(MEM_NODE_GET_SPAN(end), MEM_INVALID_SPAN);
}

// Free-list node count with a strict span-ascending check. A cycle or a
// duplicate entry shows up as a mismatch against the physical walk.
std::size_t CountFreeList() {
    MEM_HEAD_INFO *head = inner_mem_head();
    DLINK_NODE *pos = head->free_list.next;
    std::uint32_t last = 0;
    bool first = true;
    std::size_t n = 0;

    while (pos != &head->free_list && n < 4096) {
        MEM_NODE_INFO *node = reinterpret_cast<MEM_NODE_INFO *>(
            reinterpret_cast<char *>(pos) - offsetof(MEM_NODE_INFO, node_list));
        EXPECT_EQ(MEM_NODE_GET_USED(node), 0u);
        if (first) {
            first = false;
        } else {
            EXPECT_LE(last, MEM_NODE_GET_SPAN(node)) << "free list not ascending";
        }
        last = MEM_NODE_GET_SPAN(node);
        ++n;
        pos = pos->next;
    }
    return n;
}

std::size_t PhysicalFreeBlocks() {
    std::size_t n = 0;
    for (auto *b : WalkBlocks()) {
        if (MEM_NODE_GET_USED(b) == 0) {
            ++n;
        }
    }
    return n;
}

class MemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Poison the region so nothing may rely on zeroed memory.
        std::memset(g_heap, 0xA5, sizeof(g_heap));
        ASSERT_EQ(inner_mem_init(g_heap, kHeapSize), static_cast<UINT32>(OS_OK));
    }

    void TearDown() override {
        ExpectChainSane();
        EXPECT_EQ(CountFreeList(), PhysicalFreeBlocks());
    }

    static UINT32 Alloc(std::uint32_t size, void **out = nullptr) {
        void *p = nullptr;
        UINT32 rc = inner_mem_alloc(size, &p);
        if (out != nullptr) {
            *out = p;
        }
        return rc;
    }

    // 用 0x10 小块填满尾部空闲区，避免“释放顺带并入尾块”干扰合并断言。
    static void FillGaps() {
        void *p = nullptr;
        std::size_t guard = 0;
        while (inner_mem_alloc(0x10, &p) == static_cast<UINT32>(OS_OK) && guard++ < 4096) {
        }
    }

    static void *MustAlloc(std::uint32_t size) {
        void *p = nullptr;
        EXPECT_EQ(inner_mem_alloc(size, &p), static_cast<UINT32>(OS_OK));
        EXPECT_NE(p, nullptr);
        return p;
    }
};

// ---------------- T-01 / T-02: init layout ----------------

TEST_F(MemTest, T01_InitProducesSingleFreeBlock) {
    auto blocks = WalkBlocks();
    ASSERT_EQ(blocks.size(), 1u);
    EXPECT_EQ(MEM_NODE_GET_USED(blocks[0]), 0u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(blocks[0]), FirstSpan());
    EXPECT_EQ(MEM_NODE_GET_PREV_SPAN(blocks[0]), 0u);
    EXPECT_EQ(MEM_NODE_GET_PREV_SPAN(EndNode()), FirstSpan());
    EXPECT_EQ(CountFreeList(), 1u);
}

TEST_F(MemTest, T02_FirstNodeAndUserPointerAreAligned) {
    auto *first = FirstNode();
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(first) % MEM_ALIGN, 0u);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(first + 1) % MEM_ALIGN, 0u);
    EXPECT_EQ(reinterpret_cast<std::uint8_t *>(first) - g_heap,
              static_cast<std::ptrdiff_t>(MEM_POOL_INFO_SIZE + MEM_HEAD_INFO_SIZE));
    void *p = MustAlloc(0x10);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % MEM_ALIGN, 0u);
}

// ---------------- T-03..T-08: allocation basics ----------------

TEST_F(MemTest, T03_AllocRecordsHeaderPlusAlignedPayload) {
    void *p = MustAlloc(0x33);
    auto *node = NodeOf(p);
    EXPECT_NE(MEM_NODE_GET_USED(node), 0u);
    // span 口径：块头 + 对齐后的数据区，这就是“块头被算进去”的唯一证据
    EXPECT_EQ(MEM_NODE_GET_SPAN(node), SpanOf(0x33));
    EXPECT_EQ(MEM_NODE_DATA_SPAN(node), SpanOf(0x33) - MEM_NODE_INFO_SIZE);
    EXPECT_EQ(reinterpret_cast<std::uint8_t *>(p),
              reinterpret_cast<std::uint8_t *>(node) + MEM_NODE_INFO_SIZE);
}

TEST_F(MemTest, T04_AllocZeroAndOversizeRejected) {
    void *p = reinterpret_cast<void *>(0xdeadbeefu);
    EXPECT_EQ(inner_mem_alloc(0, &p), static_cast<UINT32>(MEM_SIZE_INVALID_ERR));
    EXPECT_EQ(p, nullptr);

    p = reinterpret_cast<void *>(0xdeadbeefu);
    EXPECT_EQ(inner_mem_alloc(MaxPayload() + 1, &p),
              static_cast<UINT32>(MEM_SIZE_INVALID_ERR));
    EXPECT_EQ(p, nullptr);

    // 恰好等于上限必须成功，并且整块交付（span 恰为整堆剩余）
    void *q = MustAlloc(MaxPayload());
    EXPECT_EQ(MEM_NODE_GET_SPAN(NodeOf(q)), FirstSpan());
}

TEST_F(MemTest, T05_AllocNullOutRejected) {
    EXPECT_EQ(inner_mem_alloc(0x10, nullptr), static_cast<UINT32>(MEM_IS_NULL_ERR));
}

TEST_F(MemTest, T06_InitNullPoolRejected) {
    EXPECT_EQ(inner_mem_init(nullptr, kHeapSize), static_cast<UINT32>(MEM_IS_NULL_ERR));
}

TEST_F(MemTest, T07_ExhaustionReturnsNoFreeBlockThenRecovers) {
    std::vector<void *> held;
    UINT32 rc = static_cast<UINT32>(OS_OK);
    void *p = nullptr;

    while (held.size() < 4096) {
        rc = Alloc(0x100, &p);
        if (rc != static_cast<UINT32>(OS_OK)) {
            break;
        }
        held.push_back(p);
    }
    EXPECT_EQ(rc, static_cast<UINT32>(MEM_NO_FREE_BLK_ERR));
    EXPECT_EQ(p, nullptr);

    for (void *q : held) {
        EXPECT_EQ(inner_mem_free(q), static_cast<UINT32>(OS_OK));
    }
    // 全量释放后必须回到 init 形态：单块、span 等于初始值
    auto blocks = WalkBlocks();
    ASSERT_EQ(blocks.size(), 1u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(blocks[0]), FirstSpan());
}

TEST_F(MemTest, T08_FullBlockDeliveryWhenRemainderTooSmall) {
    // 剩下的 span 不足 MEM_MIN_BLOCK 时不切分（0x10 < 0x20），整块交付
    void *p = MustAlloc(MaxPayload() - 0x10);
    auto *node = NodeOf(p);
    EXPECT_EQ(MEM_NODE_GET_SPAN(node), FirstSpan());
    EXPECT_EQ(MEM_NODE_NEXT(node), EndNode());
    for (auto *b : WalkBlocks()) {
        EXPECT_GE(MEM_NODE_GET_SPAN(b), MEM_MIN_BLOCK);
    }
}

// ---------------- T-09..T-12: free & coalescing ----------------

TEST_F(MemTest, T09_FreeMergesWithNextOnly) {
    void *a = MustAlloc(0x30);
    void *b = MustAlloc(0x30);
    void *c = MustAlloc(0x30);
    FillGaps();

    // 先释放 b（前后都占用），再释放 c：c 的后继已占用，只能与 b 合并
    EXPECT_EQ(inner_mem_free(b), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(inner_mem_free(c), static_cast<UINT32>(OS_OK));

    auto *nb = NodeOf(b);
    EXPECT_EQ(MEM_NODE_GET_USED(nb), 0u);
    // span 直接相加：每块 = 块头 + 0x30，合并后块头只保留一份
    EXPECT_EQ(MEM_NODE_GET_SPAN(nb), SpanOf(0x30) * 2u);
    EXPECT_NE(MEM_NODE_GET_USED(NodeOf(a)), 0u);
}

TEST_F(MemTest, T10_FreeMergesWithPreviousOnly) {
    void *a = MustAlloc(0x30);
    void *b = MustAlloc(0x30);
    void *c = MustAlloc(0x30);
    FillGaps();

    // a 独立入链；此后释放 b：后继 c 占用、前邻 a 空闲 → 只向前合并，合并块以 a 为首
    EXPECT_EQ(inner_mem_free(a), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(inner_mem_free(b), static_cast<UINT32>(OS_OK));

    auto *na = NodeOf(a);
    EXPECT_EQ(MEM_NODE_GET_USED(na), 0u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(na), SpanOf(0x30) * 2u);
    EXPECT_NE(MEM_NODE_GET_USED(NodeOf(c)), 0u);
}

TEST_F(MemTest, T11_FreeWithoutNeighboursGoesToFreeList) {
    void *a = MustAlloc(0x40);
    void *b = MustAlloc(0x40);
    void *c = MustAlloc(0x40);

    EXPECT_EQ(inner_mem_free(b), static_cast<UINT32>(OS_OK));
    auto *nb = NodeOf(b);
    EXPECT_EQ(MEM_NODE_GET_USED(nb), 0u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(nb), SpanOf(0x40));
    EXPECT_EQ(CountFreeList(), PhysicalFreeBlocks());
    (void)a;
    (void)c;
}

TEST_F(MemTest, T12_ThreeWayMerge) {
    void *a = MustAlloc(0x30);
    void *b = MustAlloc(0x30);
    void *c = MustAlloc(0x30);
    FillGaps();

    EXPECT_EQ(inner_mem_free(b), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(inner_mem_free(c), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(inner_mem_free(a), static_cast<UINT32>(OS_OK));

    auto *na = NodeOf(a);
    EXPECT_EQ(MEM_NODE_GET_USED(na), 0u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(na), SpanOf(0x30) * 3u);
    ExpectChainSane();
    EXPECT_EQ(CountFreeList(), PhysicalFreeBlocks());
}

TEST_F(MemTest, T13_FirstAndTailBoundariesDoNotOverreach) {
    // 释放首块只探测后邻；释放堆尾前最后一块时后继为 end_node，不合并
    void *a = MustAlloc(0x30);
    void *b = MustAlloc(MaxPayload() - SpanOf(0x30));

    EXPECT_EQ(inner_mem_free(a), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(inner_mem_free(b), static_cast<UINT32>(OS_OK));
    auto blocks = WalkBlocks();
    ASSERT_EQ(blocks.size(), 1u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(blocks[0]), FirstSpan());
}

TEST_F(MemTest, T14_DoubleFreeReturnsCode) {
    void *p = MustAlloc(0x40);
    EXPECT_EQ(inner_mem_free(p), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(inner_mem_free(p), static_cast<UINT32>(MEM_DOUBLE_FREE_ERR));
    // 二次释放只返回错误码：块仍为“空闲且只入链一次”，堆不崩
    EXPECT_EQ(MEM_NODE_GET_USED(NodeOf(p)), 0u);
    EXPECT_EQ(CountFreeList(), PhysicalFreeBlocks());
}

TEST_F(MemTest, T15_FreeNullAndBadPointers) {
    EXPECT_EQ(inner_mem_free(nullptr), static_cast<UINT32>(MEM_IS_NULL_ERR));

    void *p = MustAlloc(0x40);
    // 未对齐地址：块内偏移 1 字节（宿主 8 B、目标机 16 B 都必然失配）
    EXPECT_EQ(inner_mem_free(static_cast<std::uint8_t *>(p) + 1),
              static_cast<UINT32>(MEM_NODE_CORRUPTED_ERR));
    // 指向块头之前（堆管理区）：不属于用户数据区
    EXPECT_EQ(inner_mem_free(reinterpret_cast<void *>(FirstNode())),
              static_cast<UINT32>(MEM_BOUNDARY_OVERFLOW_ERR));
    // 指向堆尾之外
    EXPECT_EQ(inner_mem_free(reinterpret_cast<void *>(inner_mem_heap_end())),
              static_cast<UINT32>(MEM_BOUNDARY_OVERFLOW_ERR));
    EXPECT_EQ(inner_mem_free(p), static_cast<UINT32>(OS_OK));
}

// ---------------- T-16..T-19: best-fit, ordering, integrity ----------------

TEST_F(MemTest, T16_BestFitReusesExactHoleBeforeLargerBlock) {
    void *a = MustAlloc(0x40);
    void *b = MustAlloc(0x80);
    void *c = MustAlloc(0x40);
    (void)c;

    EXPECT_EQ(inner_mem_free(b), static_cast<UINT32>(OS_OK));
    void *again = MustAlloc(0x80);
    EXPECT_EQ(again, b);
    (void)a;
}

TEST_F(MemTest, T17_SplitRemainderIsFreeAndCorrectlyLinked) {
    void *p = MustAlloc(0x100);
    auto *node = NodeOf(p);
    auto *rem = MEM_NODE_NEXT(node);

    EXPECT_EQ(MEM_NODE_GET_SPAN(node), SpanOf(0x100));
    EXPECT_EQ(MEM_NODE_GET_USED(rem), 0u);
    EXPECT_EQ(MEM_NODE_GET_PREV_SPAN(rem), SpanOf(0x100));
    EXPECT_EQ(reinterpret_cast<std::uint8_t *>(rem),
              reinterpret_cast<std::uint8_t *>(node) + SpanOf(0x100));
    // rem 自身也是一整块，且 span 与整堆剩余严格互补
    EXPECT_EQ(MEM_NODE_GET_SPAN(rem), FirstSpan() - SpanOf(0x100));
}

TEST_F(MemTest, T18_FreeListStaysSizeAscending) {
    std::vector<void *> held = {MustAlloc(0x40), MustAlloc(0x80), MustAlloc(0x120)};
    for (void *p : held) {
        EXPECT_EQ(inner_mem_free(p), static_cast<UINT32>(OS_OK));
    }
    // 三块被相邻合并成一块，再要求顺序性自然成立
    ExpectChainSane();
    EXPECT_EQ(CountFreeList(), PhysicalFreeBlocks());

    // 制造三个互不相邻的空闲块，逐个检查升序
    void *a = MustAlloc(0x30);
    void *guard1 = MustAlloc(0x10);
    void *b = MustAlloc(0x60);
    void *guard2 = MustAlloc(0x10);
    void *c = MustAlloc(0x100);
    (void)guard1;
    (void)guard2;
    (void)c;
    EXPECT_EQ(inner_mem_free(b), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(inner_mem_free(a), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(CountFreeList(), PhysicalFreeBlocks());
}

TEST_F(MemTest, T19_FragmentationRecoversAfterFullRelease) {
    std::vector<void *> held;
    for (std::uint32_t sz : {0x20, 0x40, 0x80, 0x100}) {
        held.push_back(MustAlloc(sz));
    }
    for (void *p : held) {
        EXPECT_EQ(inner_mem_free(p), static_cast<UINT32>(OS_OK));
    }
    auto blocks = WalkBlocks();
    ASSERT_EQ(blocks.size(), 1u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(blocks[0]), FirstSpan());
    EXPECT_EQ(MEM_NODE_GET_PREV_SPAN(EndNode()), FirstSpan());
}

// ---------------- T-21: defensive rem merge after split ----------------

TEST_F(MemTest, T21_RemMergesWithAdjacentFreeBlock) {
    // 白盒构造非法中间态：[free A][free B][used C][end]
    // 正常路径下这种状态不可达，这里直接改块头模拟，用于验证分配侧的
    // 防御性归并分支（设计文档 5.2 / I7）。span 口径下块头已含在 span 内，
    // 手工搭建只需按 span 累加，宿主与目标机同一套期望。
    MEM_HEAD_INFO *head = inner_mem_head();
    MEM_NODE_INFO *first = FirstNode();
    MEM_NODE_INFO *end = EndNode();
    const std::uint32_t kA = 0x60;      // 空闲块 A 的 span
    const std::uint32_t kB = 0x70;      // 空闲块 B 的 span
    const std::uint32_t kReq = 0x10;    // 申请数据区：两种平台都已对齐
    const std::uint32_t kNeed = MEM_NODE_INFO_SIZE + kReq;

    MEM_NODE_INFO *b = reinterpret_cast<MEM_NODE_INFO *>(
        reinterpret_cast<std::uint8_t *>(first) + kA);
    MEM_NODE_INFO *c = reinterpret_cast<MEM_NODE_INFO *>(
        reinterpret_cast<std::uint8_t *>(b) + kB);
    std::uint32_t c_span = FirstSpan() - kA - kB;

    first->size_and_used = kA;                 // used = 0
    MEM_NODE_SET_PREV_SPAN(first, 0);
    b->size_and_used = kB;                     // used = 0
    MEM_NODE_SET_PREV_SPAN(b, kA);
    c->size_and_used = c_span | MEM_USED_MASK;
    MEM_NODE_SET_PREV_SPAN(c, kB);
    MEM_NODE_SET_PREV_SPAN(end, c_span);

    // 重建升序空闲链：A -> B
    dlink_init(&head->free_list);
    dlink_insert_tail(&head->free_list, &first->node_list);
    dlink_insert_tail(&first->node_list, &b->node_list);

    ASSERT_EQ(CountFreeList(), 2u);
    ASSERT_GE(kA - kNeed, MEM_MIN_BLOCK) << "场景前提：A 切开后 rem 仍是一整块";

    // best-fit 取 A；切开后 rem 的后邻 B 空闲 → 必须归并，否则破坏 I7
    void *p = nullptr;
    ASSERT_EQ(inner_mem_alloc(kReq, &p), static_cast<UINT32>(OS_OK));
    EXPECT_EQ(NodeOf(p), first);
    EXPECT_EQ(MEM_NODE_GET_SPAN(first), kNeed);

    MEM_NODE_INFO *rem = MEM_NODE_NEXT(first);
    const std::uint32_t rem_span = kA - kNeed;
    EXPECT_EQ(reinterpret_cast<std::uint8_t *>(rem),
              reinterpret_cast<std::uint8_t *>(first) + kNeed);
    EXPECT_EQ(MEM_NODE_GET_USED(rem), 0u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(rem), rem_span + kB);
    EXPECT_EQ(MEM_NODE_GET_PREV_SPAN(c), rem_span + kB);

    // 空闲链只剩 rem，B 已出链
    EXPECT_EQ(CountFreeList(), 1u);
    ExpectChainSane();
}

// ---------------- T-22: I7 after random workload ----------------

TEST_F(MemTest, T22_NoAdjacentFreeBlocksUnderRandomWorkload) {
    std::mt19937 rng(20261123u);
    std::map<void *, std::uint32_t> live;

    for (int i = 0; i < 1000; ++i) {
        bool do_alloc = live.empty() || (rng() % 100) < 65;
        if (do_alloc) {
            std::uint32_t sz = 1 + (rng() % 0x300);
            void *p = nullptr;
            if (inner_mem_alloc(sz, &p) == static_cast<UINT32>(OS_OK)) {
                ASSERT_EQ(reinterpret_cast<std::uintptr_t>(p) % MEM_ALIGN, 0u);
                live[p] = sz;
            }
        } else {
            auto it = live.begin();
            std::advance(it, rng() % live.size());
            EXPECT_EQ(inner_mem_free(it->first), static_cast<UINT32>(OS_OK));
            live.erase(it);
        }
    }
    for (auto &kv : live) {
        EXPECT_EQ(inner_mem_free(kv.first), static_cast<UINT32>(OS_OK));
    }
    ExpectChainSane();
    auto blocks = WalkBlocks();
    ASSERT_EQ(blocks.size(), 1u);
    EXPECT_EQ(MEM_NODE_GET_SPAN(blocks[0]), FirstSpan());
}

// ---------------- T-25: release does not zero ----------------

TEST_F(MemTest, T25_FreeKeepsPayloadUntouched) {
    void *p = MustAlloc(0x80);
    std::memset(p, 0x5A, 0x80);
    EXPECT_EQ(inner_mem_free(p), static_cast<UINT32>(OS_OK));

    void *q = MustAlloc(0x80);
    EXPECT_EQ(q, p);
    std::uint8_t *data = static_cast<std::uint8_t *>(q);
    for (int i = 0; i < 0x80; ++i) {
        ASSERT_EQ(data[i], 0x5A) << "payload byte " << i << " was modified by free()";
    }
}

// ---------------- Shadow model: overlap / leak detection ----------------

class ShadowModel {
public:
    ShadowModel() : units_((FirstSpan() - MEM_NODE_INFO_SIZE) / MEM_ALIGN, 0) {}

    void Mark(void *p, std::uint32_t size) {
        std::uint32_t need = (size + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1);
        std::uint32_t idx = IndexOf(p);
        for (std::uint32_t i = 0; i < need / MEM_ALIGN; ++i) {
            EXPECT_EQ(units_[idx + i], 0u) << "overlapping allocation";
            units_[idx + i] = 1;
        }
    }

    void Unmark(void *p, std::uint32_t size) {
        std::uint32_t need = (size + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1);
        std::uint32_t idx = IndexOf(p);
        for (std::uint32_t i = 0; i < need / MEM_ALIGN; ++i) {
            EXPECT_EQ(units_[idx + i], 1u) << "double free in shadow model";
            units_[idx + i] = 0;
        }
    }

    std::uint32_t LiveUnits() const {
        std::uint32_t n = 0;
        for (auto u : units_) {
            n += u;
        }
        return n;
    }

private:
    std::uint32_t IndexOf(void *p) const {
        std::uint8_t *base = reinterpret_cast<std::uint8_t *>(FirstNode()) + MEM_NODE_INFO_SIZE;
        return static_cast<std::uint32_t>((static_cast<std::uint8_t *>(p) - base) / MEM_ALIGN);
    }

    std::vector<std::uint8_t> units_;
};

TEST_F(MemTest, T20_ShadowModelDetectsOverlapAndLeaks) {
    ShadowModel shadow;
    std::mt19937 rng(4242u);
    std::map<void *, std::uint32_t> live;

    for (int i = 0; i < 1000; ++i) {
        if (live.empty() || (rng() % 100) < 60) {
            std::uint32_t sz = 1 + (rng() % 0x200);
            void *p = nullptr;
            if (inner_mem_alloc(sz, &p) == static_cast<UINT32>(OS_OK)) {
                shadow.Mark(p, sz);
                live[p] = sz;
            }
        } else {
            auto it = live.begin();
            std::advance(it, rng() % live.size());
            shadow.Unmark(it->first, it->second);
            EXPECT_EQ(inner_mem_free(it->first), static_cast<UINT32>(OS_OK));
            live.erase(it);
        }
    }
    for (auto &kv : live) {
        shadow.Unmark(kv.first, kv.second);
        EXPECT_EQ(inner_mem_free(kv.first), static_cast<UINT32>(OS_OK));
    }
    EXPECT_EQ(shadow.LiveUnits(), 0u) << "shadow model still holds live units";
    ExpectChainSane();
    auto blocks = WalkBlocks();
    ASSERT_EQ(blocks.size(), 1u);
}

}  // namespace
