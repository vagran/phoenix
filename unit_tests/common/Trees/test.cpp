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

UT_TEST("RB tree")
{
    TestItem::TestTree tree;

    const size_t numItems = 8192;
    TestItem items[numItems];

    for (size_t i = 0; i < numItems; i++) {
        TestItem &item = items[i];
        item.idx = i;
        UT(tree.Insert(&item, &item._rbEntry)) == UT(&item);
        item.inserted = true;
    }

    /* The second insertion should not succeed. */
    TestItem tmpItems[numItems];
    for (size_t i = 0; i < numItems; i++) {
        TestItem &item = tmpItems[i];
        item.idx = i;
        UT(tree.Insert(&item, &item._rbEntry)) == UT_NULL;
        item.inserted = true;
    }

    /* Verify tree iteration. */
    for (TestItem &item: tree) {
        UT(item.visited) == UT(false);
        item.visited = true;
    }

    for (size_t i = 0; i < numItems; i++) {
        TestItem &item = items[i];
        if (item.inserted) {
            UT(item.visited) == UT(true);
        }
        item.visited = false;
    }

    UT(tree.Validate()) == UT(true);

    /* Verify lookups */
    for (size_t i = 0; i < numItems; i++) {
        TestItem *item = tree.Lookup(i);
        UT(item) != UT_NULL;
        UT(item->idx) == UT(i);
    }

    //notimpl
}
UT_TEST_END
