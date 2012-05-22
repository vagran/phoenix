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

UT_TEST("Basic lists operations")
{
    List<int> l;
    //int l_v[] = { 1, 2, 3, 4, 5, 6 };

    UT(len(l)) == UT_SIZE(0);
    /* Iterate empty list. */
    //XXX
    l.append(2);
    UT(len(l)) == UT_SIZE(1);
    UT(l[0]) == UT(2);
    UT(l[-1]) == UT(2);
    /* Iterate one element list. */
    //XXX
    l.append(5);
    UT(l[-1]) == UT(5);
    l.insert(10, 6);
    UT(l[-1]) == UT(6);
    UT(l[-2]) == UT(5);
    UT(l[-3]) == UT(2);
    UT(l[0]) == UT(2);
    UT(l[1]) == UT(5);
    UT(l[2]) == UT(6);
    l.insert(-10, 2);
    l.insert(1, 4);
    l.insert(1, 3);
    UT(len(l)) == UT_SIZE(6);

    /* Initializer list construction and assignment. */
    //XXX

    List<Numeric<int> *> l2;
    UT(len(l2)) == UT_SIZE(0);
    l2.append(NEW Numeric<int>(1));
    UT(len(l2)) == UT_SIZE(1);
    UT(static_cast<int>(*l2[0])) == UT(1);
}
UT_TEST_END
