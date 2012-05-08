/*
 * /phoenix/unit_tests/triton/generic/test.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Tests for Triton generic functionality.
 */

#include <phoenix_ut.h>

#include <sys.h>

using namespace triton;

UT_TEST("Objects pointers")
{
    Object o1;
    Ptr<Object> p1(o1);
    Ptr<Object> p2(NEW Object);
    Ptr<Object> p3(p2);
    Ptr<Object> p4;

    UT(p1 == p2) == UT_FALSE;
    UT(p1 != p2) == UT_TRUE;
    UT(p2 == p3) == UT_TRUE;
    UT(p2 != p3) == UT_FALSE;
    UT_BOOL(p2) == UT_TRUE;
    UT_BOOL(p4) == UT_FALSE;

    p4 = p2;
    UT(p4 == p3) == UT_TRUE;
    UT_BOOL(p4) == UT_TRUE;
    p2 = p1;

    p4->AddRef();
    (*p4).Release();
    p4 = nullptr;
    UT_BOOL(p4) == UT_FALSE;

    bool catched = false;
    try {
        p4->AddRef();
    } catch (NullPtrError &) {
        catched = true;
    }
    UT(catched) == UT_TRUE;
}
UT_TEST_END

UT_TEST("Tuples")
{
    Tuple<int, const char *, float> t(237, "test", 2.0);
    UT(len(t)) == UT(static_cast<size_t>(3));

    /* Retrieving components types. */
    decltype(t)::Type<0> v0 = 237;
    decltype(t)::Type<1> v1 = "test";
    decltype(t)::Type<2> v2 = 2.0;
    /* Retrieving components values. */
    UT(t.get<0>()) == UT(v0);
    UT(t.get<1>()) == UT(v1);
    UT(t.get<2>()) == UT(v2);
}
UT_TEST_END
