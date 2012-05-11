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

UT_TEST("Helper utilities")
{
    UT((ice_or<true, false, true, false>())) == UT_TRUE;
    UT((ice_or<false, false, false, false>())) == UT_FALSE;
    UT((ice_or<true, true, true, true>())) == UT_TRUE;
    UT((ice_not<ice_or<false, false, false, false>()>())) == UT_TRUE;

    UT((ice_and<true, false, true, false>())) == UT_FALSE;
    UT((ice_and<false, false, false, false>())) == UT_FALSE;
    UT((ice_and<true, true, true, true>())) == UT_TRUE;
    UT((ice_not<ice_and<false, false, false, false>()>())) == UT_TRUE;

    enable_if<ice_or<true, false, true, false>(), int> i = 0;
    UT(i) == UT(0);
}
UT_TEST_END

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
    typedef Tuple<int, char /* const char * */, float> tuple_t;
    tuple_t t(237, 10/* "test" */, 2.0);
    UT(len(t)) == UT(static_cast<size_t>(3));
    UT(len(tuple_t(237, 10 /* "test" */, 2.0))) == UT(static_cast<size_t>(3));
    Ptr<tuple_t> p(t);
    UT(len(p)) == UT(static_cast<size_t>(3));
    UT(len(Ptr<tuple_t>(t))) == UT(static_cast<size_t>(3));

    /* Retrieving components types. */
    decltype(t)::Type<0> v0 = 237;
    //decltype(t)::Type<1> v1 = "test";
    decltype(t)::Type<1> v1 = 10;
    decltype(t)::Type<2> v2 = 2.0;
    /* Retrieving components values. */
    UT(t.get<0>()) == UT(v0);
    UT(t.get<1>()) == UT(v1);
    UT(t.get<2>()) == UT(v2);

    //tuple_t t2(t);
    //UT(hash(t2)) != UT(hash(t));
}
UT_TEST_END

UT_TEST("Numeric values")
{
    Int i1(1);
    Int i2 = 2;

    UT(static_cast<int>(i1)) == UT(1);
    UT(static_cast<int>(i2)) == UT(2);
    UT(i1 == 1) == UT_TRUE;
    UT(i1 == 1.0) == UT_TRUE;
    UT(i1 == i2) == UT_FALSE;
    UT(i1 != i2) == UT_TRUE;
    UT(i1 < 2) == UT_TRUE;
    UT(i1 < 2.0) == UT_TRUE;
    UT(i1 < 0) == UT_FALSE;
    UT(i1 > 0.5) == UT_TRUE;
    UT(i1 < i2) == UT_TRUE;

    Object::hash_t h1 = 1, h2 = 2;
    UT(hash(i1)) == UT(h1);
    UT(hash(i2)) == UT(h2);
    UT(hash(1)) == UT(h1);
    UT(hash(2)) == UT(h2);

    i1 = 7;
    UT(i1 == 7) == UT_TRUE;
    i1 &= 0x13;
    UT(i1 == 3) == UT_TRUE;

    i1 = 7;
    i1 &= Int(0x13);
    UT(i1 == 3) == UT_TRUE;

    i1 = 1;
    UT(~i1) == UT(~1);
}
UT_TEST_END
