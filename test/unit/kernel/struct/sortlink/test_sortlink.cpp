// Unit tests for OSZ's sorted doubly-linked list (`sortlink`).
//
// The OSZ sortlink keeps entries in *ascending absolute timeout order*,
// but each entry stores its *delta* from the previous entry's fire time
// (the head sentinel holds the special "infinity" timeout). This is the
// classic O(1)-per-tick software-timer representation.
//
// Insertion semantics (verified by these tests):
//   - Walking the dlist runs from head.next through to head again.
//     In OSZ's dlink, that path corresponds to earliest-fire-first.
//   - Inserting in *descending* timeout order (largest timeout inserted
//     first) yields a list where each entry's stored delta equals the
//     gap from the previous entry, and the order is exactly the desired
//     fire order: smallest timeout first.
//   - The implementation REJECTS a zero timeout (`SORTLINK_TIMEOUT_INVALID`).
//   - `sortlink_delete` adds the deleted entry's delta back to the next
//     entry (so a "gap" closes up).

#include <gtest/gtest.h>

extern "C" {
#include "sortlink.h"
#include "dlink.h"
#include "inner_sortlink_err.h"
}

namespace {

// Walk the dlist from head.next, copying each SORT_LINK pointer into out[].
// Returns the number of entries collected.
static int collect(const SORT_LINK &head, SORT_LINK *out[], int max) {
    int n = 0;
    DLINK_NODE *iter = nullptr;
    DLINK_FOREACH(iter, &PSORTLINK_LIST(&head)) {
        if (n >= max) break;
        out[n++] = STRUCT_ENTRY(SORT_LINK, list, iter);
    }
    return n;
}

TEST(SortLink, InitSetsInfinityTimeoutOnHead) {
    SORT_LINK head{};
    sortlink_init(&head);
    EXPECT_EQ(PSORTLINK_TIMEOUT(&head), 0xFFFFFFFFu);
    EXPECT_EQ(DLINK_EMPTY(&PSORTLINK_LIST(&head)), 1);
}

TEST(SortLink, InsertSingleEntry) {
    SORT_LINK head{};
    sortlink_init(&head);

    // NOTE: do NOT call sortlink_init on `a` — init overwrites the
    // timeout with SORTLINK_MAX_TIMEOUT. The OSZ sortlink only requires
    // the head to be initialized; leaf entries just need their timeout
    // and an empty dlist (which default-construction gives us).
    SORT_LINK a{};
    PSORTLINK_TIMEOUT(&a) = 7;

    EXPECT_EQ(sortlink_insert(&head, &a), OS_OK);

    SORT_LINK *items[4] = {nullptr, nullptr, nullptr, nullptr};
    int n = collect(head, items, 4);
    ASSERT_EQ(n, 1);
    EXPECT_EQ(items[0], &a);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[0]), 7u);
}

TEST(SortLink, InsertDescendingTimeoutPreservesDeltas) {
    // Inserting larger timeouts first lets each new entry land at the
    // front of the iteration, with its stored timeout equal to the gap
    // from the next entry's fire time.
    SORT_LINK head{};
    sortlink_init(&head);

    SORT_LINK a{}, b{}, c{};
    PSORTLINK_TIMEOUT(&a) = 1;
    PSORTLINK_TIMEOUT(&b) = 3;
    PSORTLINK_TIMEOUT(&c) = 5;

    ASSERT_EQ(sortlink_insert(&head, &c), OS_OK);  // 5
    ASSERT_EQ(sortlink_insert(&head, &b), OS_OK);  // 3
    ASSERT_EQ(sortlink_insert(&head, &a), OS_OK);  // 1

    SORT_LINK *items[4] = {nullptr};
    int n = collect(head, items, 4);
    ASSERT_EQ(n, 3);

    // Fire order = a (1), b (3), c (5).
    EXPECT_EQ(items[0], &a);
    EXPECT_EQ(items[1], &b);
    EXPECT_EQ(items[2], &c);

    // Deltas: a fires 1 tick from now, b fires 2 ticks after a,
    // c fires 2 ticks after b. Cumulative: 1, 3, 5.
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[0]), 1u);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[1]), 2u);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[2]), 2u);
}

TEST(SortLink, InsertAscendingTimeoutProducesUnusualDeltas) {
    // Document the actual behaviour: inserting smaller timeouts first
    // walks to the end of the dlist and accumulates deltas along the
    // way, but the LIST ends up in ascending absolute-timeout order
    // (a first, c last). This is the same layout you'd get from
    // descending inserts; the difference is just which deltas each
    // entry winds up storing.
    SORT_LINK head{};
    sortlink_init(&head);

    SORT_LINK a{}, b{}, c{};
    PSORTLINK_TIMEOUT(&a) = 1;
    PSORTLINK_TIMEOUT(&b) = 3;
    PSORTLINK_TIMEOUT(&c) = 5;

    ASSERT_EQ(sortlink_insert(&head, &a), OS_OK);  // 1
    ASSERT_EQ(sortlink_insert(&head, &b), OS_OK);  // 3
    ASSERT_EQ(sortlink_insert(&head, &c), OS_OK);  // 5

    SORT_LINK *items[4] = {nullptr};
    int n = collect(head, items, 4);
    ASSERT_EQ(n, 3);

    // Entries are sorted by absolute timeout ascending.
    EXPECT_EQ(items[0], &a);
    EXPECT_EQ(items[1], &b);
    EXPECT_EQ(items[2], &c);

    // Stored deltas (each is the gap from the previous entry's fire
    // time):
    //   a: stored timeout unchanged = 1.
    //   b: subtractions along the walk -> 3 - a.delta(1) = 2.
    //   c: subtractions along the walk -> 5 - a.delta(1) - b.delta(2) = 2.
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[0]), 1u);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[1]), 2u);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[2]), 2u);
}

TEST(SortLink, RejectsZeroTimeout) {
    SORT_LINK head{};
    sortlink_init(&head);

    SORT_LINK a{};
    PSORTLINK_TIMEOUT(&a) = 0;

    EXPECT_EQ(sortlink_insert(&head, &a), SORTLINK_TIMEOUT_INVALID);
    EXPECT_EQ(DLINK_EMPTY(&PSORTLINK_LIST(&head)), 1);
}

TEST(SortLink, DeleteMiddleClosesGap) {
    SORT_LINK head{};
    sortlink_init(&head);

    SORT_LINK a{}, b{}, c{};
    PSORTLINK_TIMEOUT(&a) = 1;
    PSORTLINK_TIMEOUT(&b) = 3;
    PSORTLINK_TIMEOUT(&c) = 5;

    sortlink_insert(&head, &c);  // 5
    sortlink_insert(&head, &b);  // 3
    sortlink_insert(&head, &a);  // 1

    // Layout: head -> a(1) -> b(2) -> c(2) -> head.
    ASSERT_EQ(PSORTLINK_TIMEOUT(&b), 2u);

    EXPECT_EQ(sortlink_delete(&b), OS_OK);

    SORT_LINK *items[4] = {nullptr};
    int n = collect(head, items, 4);
    ASSERT_EQ(n, 2);
    EXPECT_EQ(items[0], &a);
    EXPECT_EQ(items[1], &c);

    // a's delta is unchanged.
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[0]), 1u);
    // c absorbs b's old delta: was 2, plus b's 2 -> 4.
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[1]), 4u);
}

TEST(SortLink, DeleteHeadEntry) {
    SORT_LINK head{};
    sortlink_init(&head);

    SORT_LINK a{}, b{}, c{};
    PSORTLINK_TIMEOUT(&a) = 1;
    PSORTLINK_TIMEOUT(&b) = 3;
    PSORTLINK_TIMEOUT(&c) = 5;

    sortlink_insert(&head, &c);
    sortlink_insert(&head, &b);
    sortlink_insert(&head, &a);

    // Delete a (the entry that fires first).
    // sortlink_delete absorbs the deleted entry's delta into its
    // successor. Before delete: a.delta=1, b.delta=2, c.delta=2.
    // After: b.delta becomes 1 + 2 = 3, c.delta stays 2.
    EXPECT_EQ(sortlink_delete(&a), OS_OK);

    SORT_LINK *items[4] = {nullptr};
    int n = collect(head, items, 4);
    ASSERT_EQ(n, 2);
    EXPECT_EQ(items[0], &b);
    EXPECT_EQ(items[1], &c);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[0]), 3u);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[1]), 2u);
}

TEST(SortLink, DeleteTailEntry) {
    SORT_LINK head{};
    sortlink_init(&head);

    SORT_LINK a{}, b{}, c{};
    PSORTLINK_TIMEOUT(&a) = 1;
    PSORTLINK_TIMEOUT(&b) = 3;
    PSORTLINK_TIMEOUT(&c) = 5;

    sortlink_insert(&head, &c);
    sortlink_insert(&head, &b);
    sortlink_insert(&head, &a);

    // Delete c (the entry that fires last).
    EXPECT_EQ(sortlink_delete(&c), OS_OK);

    SORT_LINK *items[4] = {nullptr};
    int n = collect(head, items, 4);
    ASSERT_EQ(n, 2);
    EXPECT_EQ(items[0], &a);
    EXPECT_EQ(items[1], &b);
    // No successor exists, so nothing needs to absorb c's delta.
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[0]), 1u);
    EXPECT_EQ(PSORTLINK_TIMEOUT(items[1]), 2u);
}

}  // namespace