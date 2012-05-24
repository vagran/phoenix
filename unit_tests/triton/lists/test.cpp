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
CheckList(List<T> &list, const initializer_list<T> &il)
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
    l.insert(2, 4);
    CheckList(l, {1, 2, 4, 5, 6});
    l.insert(2, 3);
    CheckList(l, {1, 2, 3, 4, 5, 6});

    //XXX copy constructor, move constructor

    /* Initializer list construction and assignment. */
    //XXX

    List<Numeric<int> *> l2;
    UT(len(l2)) == UT_SIZE(0);
    l2.append(NEW Numeric<int>(1));
    UT(len(l2)) == UT_SIZE(1);
    UT(static_cast<int>(*l2[0])) == UT(1);
}
UT_TEST_END
