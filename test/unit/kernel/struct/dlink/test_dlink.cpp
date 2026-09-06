// Unit tests for OSZ's circular doubly-linked list (`dlink`).
//
// The OSZ dlink is a sentinel-based circular list. `dlink_init(head)`
// makes the head point to itself. `DLINK_FOREACH` iterates from
// `head->next` until it reaches `head` again.
//
// Note: the function names use the *opposite* convention from what one
// might expect from a non-circular list. The actual behaviour is:
//   - `dlink_insert_tail(head, n)` inserts `n` *right after* the head
//     sentinel — i.e. at the FRONT of `DLINK_FOREACH` iteration.
//   - `dlink_insert_head(head, n)` inserts `n` *right before* the head
//     sentinel — i.e. at the BACK of `DLINK_FOREACH` iteration.
// The tests below validate this actual behaviour.

#include <gtest/gtest.h>

extern "C" {
#include "dlink.h"
}

namespace {

// Helper: a tracked item embedded in a dlink node.
struct Item {
    DLINK_NODE link;
    int        value;
};

static Item *entry_of(DLINK_NODE *node) {
    return reinterpret_cast<Item *>(
        reinterpret_cast<char *>(node) - offsetof(Item, link));
}

TEST(DLink, InitProducesEmptySentinel) {
    DLINK_NODE head;
    // Pre-poison to make sure init actually resets the pointers.
    head.next = reinterpret_cast<DLINK_NODE *>(0xdeadbeef);
    head.pre  = reinterpret_cast<DLINK_NODE *>(0xdeadbeef);

    dlink_init(&head);

    EXPECT_EQ(DLINK_GET_NEXT(&head), &head);
    EXPECT_EQ(DLINK_GET_PRE(&head),  &head);
    EXPECT_EQ(DLINK_EMPTY(&head), 1);
}

TEST(DLink, InsertTailPutsNodeRightAfterHead) {
    DLINK_NODE head;
    dlink_init(&head);

    Item a{};
    a.value = 42;

    dlink_insert_tail(&head, &a.link);

    EXPECT_FALSE(DLINK_EMPTY(&head));
    EXPECT_EQ(DLINK_GET_NEXT(&head), &a.link);
    EXPECT_EQ(DLINK_GET_PRE(&head),  &a.link);
    EXPECT_EQ(DLINK_GET_NEXT(&a.link), &head);
    EXPECT_EQ(DLINK_GET_PRE(&a.link),  &head);
}

TEST(DLink, InsertHeadPutsNodeRightBeforeHead) {
    DLINK_NODE head;
    dlink_init(&head);

    Item a{};
    a.value = 42;

    dlink_insert_head(&head, &a.link);

    EXPECT_FALSE(DLINK_EMPTY(&head));
    EXPECT_EQ(DLINK_GET_PRE(&head),  &a.link);
    EXPECT_EQ(DLINK_GET_NEXT(&head), &a.link);
    EXPECT_EQ(DLINK_GET_NEXT(&a.link), &head);
    EXPECT_EQ(DLINK_GET_PRE(&a.link),  &head);
}

TEST(DLink, MultipleInsertsBuildIterationOrder) {
    DLINK_NODE head;
    dlink_init(&head);

    Item a{}, b{}, c{};
    a.value = 1; b.value = 2; c.value = 3;

    // Three tail-inserts. Each new node goes right after head, so the
    // most-recently-inserted node ends up first in iteration:
    //     head <-> c <-> b <-> a
    dlink_insert_tail(&head, &a.link);
    dlink_insert_tail(&head, &b.link);
    dlink_insert_tail(&head, &c.link);

    // Two head-inserts. Each new node goes right before head, so the
    // most-recently-inserted node ends up last in iteration:
    //     head <-> c <-> b <-> a <-> d <-> e
    Item d{}, e{};
    d.value = 4; e.value = 5;
    dlink_insert_head(&head, &d.link);
    dlink_insert_head(&head, &e.link);

    int seen[5] = {0};
    int idx = 0;
    DLINK_NODE *iter = nullptr;
    DLINK_FOREACH(iter, &head) {
        ASSERT_LT(idx, 5);
        seen[idx++] = entry_of(iter)->value;
    }
    EXPECT_EQ(idx, 5);
    EXPECT_EQ(seen[0], 3);
    EXPECT_EQ(seen[1], 2);
    EXPECT_EQ(seen[2], 1);
    EXPECT_EQ(seen[3], 4);
    EXPECT_EQ(seen[4], 5);
}

TEST(DLink, DelNodeUnlinksAndReturnsNode) {
    DLINK_NODE head;
    dlink_init(&head);

    Item a{}, b{}, c{};
    a.value = 1; b.value = 2; c.value = 3;
    dlink_insert_tail(&head, &a.link);
    dlink_insert_tail(&head, &b.link);
    dlink_insert_tail(&head, &c.link);
    // After the three tail-inserts:
    //     head <-> c <-> b <-> a
    //     head.next = c, head.pre = a

    DLINK_NODE *ret = dlink_del_node(&b.link);
    EXPECT_EQ(ret, &b.link);
    // OSZ resets both pointers to NULL after deletion.
    EXPECT_EQ(b.link.next, nullptr);
    EXPECT_EQ(b.link.pre,  nullptr);

    // After removing b, the list is:
    //     head <-> c <-> a
    EXPECT_EQ(head.next, &c.link);
    EXPECT_EQ(c.link.next, &a.link);
    EXPECT_EQ(a.link.next, &head);
    EXPECT_EQ(head.pre, &a.link);
    EXPECT_EQ(a.link.pre, &c.link);
    EXPECT_EQ(c.link.pre, &head);
}

TEST(DLink, DelNodeFromSingleItemList) {
    DLINK_NODE head;
    dlink_init(&head);

    Item a{};
    a.value = 9;
    dlink_insert_tail(&head, &a.link);

    dlink_del_node(&a.link);

    EXPECT_EQ(DLINK_EMPTY(&head), 1);
    EXPECT_EQ(head.next, &head);
    EXPECT_EQ(head.pre,  &head);
}

TEST(DLink, DelFirstIterNodeUpdatesHead) {
    // "First in iteration" = node immediately after head. With three
    // tail-inserts that's c, so removing c leaves head <-> b <-> a.
    DLINK_NODE head;
    dlink_init(&head);

    Item a{}, b{}, c{};
    a.value = 1; b.value = 2; c.value = 3;
    dlink_insert_tail(&head, &a.link);
    dlink_insert_tail(&head, &b.link);
    dlink_insert_tail(&head, &c.link);

    dlink_del_node(&c.link);

    EXPECT_EQ(head.next, &b.link);
    EXPECT_EQ(head.pre,  &a.link);
    EXPECT_EQ(b.link.pre, &head);
    EXPECT_EQ(a.link.next, &head);
}

TEST(DLink, DelLastIterNodeUpdatesHead) {
    // "Last in iteration" = node immediately before head. With three
    // tail-inserts that's a, so removing a leaves head <-> c <-> b.
    DLINK_NODE head;
    dlink_init(&head);

    Item a{}, b{}, c{};
    a.value = 1; b.value = 2; c.value = 3;
    dlink_insert_tail(&head, &a.link);
    dlink_insert_tail(&head, &b.link);
    dlink_insert_tail(&head, &c.link);

    dlink_del_node(&a.link);

    EXPECT_EQ(head.next, &c.link);
    EXPECT_EQ(head.pre,  &b.link);
    EXPECT_EQ(c.link.next, &b.link);
    EXPECT_EQ(b.link.pre, &c.link);
}

TEST(DLink, ForeachEmptyListDoesNothing) {
    DLINK_NODE head;
    dlink_init(&head);

    int count = 0;
    DLINK_NODE *iter = nullptr;
    DLINK_FOREACH(iter, &head) {
        (void)iter;
        ++count;
    }
    EXPECT_EQ(count, 0);
}

}  // namespace