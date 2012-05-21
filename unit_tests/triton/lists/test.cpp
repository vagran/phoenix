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

    UT(len(l)) == UT_SIZE(0);
    l.append(1);
    UT(len(l)) == UT_SIZE(1);

    List<Numeric<int> *> l2;
    UT(len(l2)) == UT_SIZE(0);
    l2.append(NEW Numeric<int>(1));
    UT(len(l2)) == UT_SIZE(1);
}
UT_TEST_END
