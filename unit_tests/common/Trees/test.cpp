/*
 * /phoenix/unit_tests/common/Trees/test.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <phoenix_ut.h>

#include <sys.h>

class TestItem {
public:
    size_t idx;
    bool inserted;
    bool visited;

    TestItem(size_t idx = 0) {
        this->idx = idx;
        inserted = false;
        visited = false;
    }

    int Compare(TestItem &item)
    {
        return idx - item.idx;
    }

    int Compare(size_t &key)
    {
        return key - idx;
    }

    typedef class RBTree<TestItem, &TestItem::Compare, size_t, &TestItem::Compare> TestTree;

    TestTree::Entry _rbEntry;
};

static void
VerifyTree(TestItem::TestTree &tree, const size_t numItems, TestItem *items)
{
    /* Verify tree iteration. */
    for (TestItem &item: tree) {
        UT(item.visited) == UT(false);
        item.visited = true;
    }

    for (size_t i = 0; i < numItems; i++) {
        TestItem &item = items[i];
        UT(item.inserted) == UT(item.visited);
        item.visited = false;
    }

    UT(tree.Validate()) == UT(true);

    /* Verify lookups */
    for (size_t i = 0; i < numItems; i++) {
        TestItem *item = tree.Lookup(i);
        if (items[i].inserted) {
            UT(item) != UT_NULL;
            UT(item->idx) == UT(i);
        } else {
            UT(item) == UT_NULL;
        }
    }
}

UT_TEST("RB tree")
{
    TestItem::TestTree tree;

    const size_t numItems = 256;
    const size_t numDeletions = 100;
    TestItem items[numItems];
    memset(items, 0, sizeof(items));

    for (size_t i = 0; i < numItems; i++) {
        items[i].idx = i;
    }

    UT_TRACE("Verifying insertions...");
    for (size_t i = 0; i < numItems; i++) {
        TestItem &item = items[i];
        UT(tree.Insert(&item, &item._rbEntry)) == UT(&item);
        item.inserted = true;
        VerifyTree(tree, numItems, items);
    }

    /* The second insertion should not succeed. */
    TestItem tmpItems[numItems];
    for (size_t i = 0; i < numItems; i++) {
        TestItem &item = tmpItems[i];
        UT(tree.Insert(&item, &item._rbEntry)) == UT_NULL;
        item.inserted = true;
    }

    VerifyTree(tree, numItems, items);

    UT_TRACE("Verifying deletions...");
    for (size_t i = 0; i < numDeletions; i++) {
        TestItem *item = tree.Delete(i);
        UT(item) != UT_NULL;
        UT(item->idx) == UT(i);
        item->inserted = false;
        VerifyTree(tree, numItems, items);
    }
    /* Delete the reset from the end. */
    for (size_t i = numItems - 1; i >= numDeletions; i--) {
        TestItem *item = tree.Delete(i);
        UT(item) != UT_NULL;
        UT(item->idx) == UT(i);
        item->inserted = false;
        VerifyTree(tree, numItems, items);
    }
}
UT_TEST_END
