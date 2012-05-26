/*
 * /phoenix/unit_tests/triton/lists/test.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for Triton lists.
 */

#include <phoenix_ut.h>

#include <sys.h>

using namespace triton;

/* Verify that list contains expected values provided in the second argument. */
template<typename T>
void
CheckList(List<T> &list, const InitList<T> &il)
{
    /* Verify length. */
    UT(len(list)) == UT(il.size());
    /* Verify direct access to all items using positive and negative indexes. */
    auto il_it = il.begin();
    for (size_t i = 0; i < il.size(); i++) {
        UT(list[i]) == UT(*il_it);
        UT(list[-il.size() + i]) == UT(*il_it);
        il_it++;
    }
    /* Verify IndexError exceptions when accessing items out of range. */
    bool catched = false;
    try {
        list[il.size()];
    } catch(IndexError &) {
        catched = true;
    }
    UT(catched) == UT_TRUE;
    catched = false;
    try {
        list[-il.size() - 1];
    } catch(IndexError &) {
        catched = true;
    }
    UT(catched) == UT_TRUE;

    /* Verify iteration. Firstly verify iter() and next() functions interface. */
    auto it = iter(list);
    il_it = il.begin();
    for (size_t i = 0; i < il.size(); i++) {
        auto &value = next(it);
        UT(value) == UT(*il_it);
        il_it++;
    }
    /* StopIteration exception should be thrown when next() called more times
     * than number of items in the list.
     */
    catched = false;
    try {
        next(it);
    } catch (StopIteration &) {
        catched = true;
    }
    UT(catched) == UT_TRUE;

    /* Verify "for"-based iteration. */
    il_it = il.begin();
    size_t i = 0;
    for (auto item: list) {
        if (i >= il.size()) {
            UT_FAIL("Number of iterated values is more than expected");
        }
        UT(item) == UT(*il_it);
        il_it++;
        i++;
    }
    UT(i) == UT(il.size());

    /* Iterating over iterator should also work. */
    il_it = il.begin();
    i = 0;
    for (auto item: iter(list)) {
        if (i >= il.size()) {
            UT_FAIL("Number of iterated values is more than expected");
        }
        UT(item) == UT(*il_it);
        il_it++;
        i++;
    }
    UT(i) == UT(il.size());
}

UT_TEST("Basic lists operations")
{
    List<int> l;

    CheckList(l, {});
    l.append(2);
    CheckList(l, {2});
    l.append(5);
    CheckList(l, {2, 5});
    l.insert(10, 6);
    CheckList(l, {2, 5, 6});
    l.insert(-10, 1);
    CheckList(l, {1, 2, 5, 6});
    Int i1 = 4;
    l.insert(2, i1);
    CheckList(l, {1, 2, 4, 5, 6});
    int i2 = 3;
    l.insert(2, i2);
    CheckList(l, {1, 2, 3, 4, 5, 6});

    /* Verify modification during iteration. */
    for (auto &item: l) {
        item = 42;
    }
    CheckList(l, {42, 42, 42, 42, 42, 42});

    /* Initializer list assignment. */
    l = {2, 3, 4};
    CheckList(l, {2, 3, 4});

    /* Object pointer values. */
    List<Int *> l2;
    UT(len(l2)) == UT_SIZE(0);
    l2.append(NEW Int(1));
    UT(len(l2)) == UT_SIZE(1);
    UT(static_cast<int>(*l2[0])) == UT(1);
    l2[0] = NEW Int(2);
    UT(len(l2)) == UT_SIZE(1);
    UT(static_cast<int>(*l2[0])) == UT(2);
    l2.append(NEW Int(3));
    UT(len(l2)) == UT_SIZE(2);
    UT(static_cast<int>(*l2[1])) == UT(3);

    /* Initializer list construction. */
    List<int> l3 {1, 2, 3, 4};
    CheckList(l3, {1, 2, 3, 4});

    /* Copy constructor. */
    List<int> l4(l3);
    CheckList(l4, {1, 2, 3, 4});
    List<Int *> l5(l2);
    UT(len(l5)) == UT_SIZE(2);
    UT(static_cast<int>(*l5[0])) == UT(2);
    UT(static_cast<int>(*l5[1])) == UT(3);

    /* Move constructor. */
    List<Int *> l6(move(l5));
    UT(len(l6)) == UT_SIZE(2);
    UT(static_cast<int>(*l6[0])) == UT(2);
    UT(static_cast<int>(*l6[1])) == UT(3);
    UT(len(l5)) == UT_SIZE(0);

    List<Int *> l7;
    l7 = l6;
    UT(len(l7)) == UT_SIZE(2);
    UT(static_cast<int>(*l7[0])) == UT(2);
    UT(static_cast<int>(*l7[1])) == UT(3);

    List<Int *> l8;
    l8 = move(l6);
    UT(len(l8)) == UT_SIZE(2);
    UT(static_cast<int>(*l8[0])) == UT(2);
    UT(static_cast<int>(*l8[1])) == UT(3);
    UT(len(l6)) == UT_SIZE(0);

    List<int> l9;
    l9 = list(l3);
    CheckList(l9, {1, 2, 3, 4});
    l9 = list({5, 6, 7});
    CheckList(l9, {5, 6, 7});

    //XXX list("abc");
}
UT_TEST_END
