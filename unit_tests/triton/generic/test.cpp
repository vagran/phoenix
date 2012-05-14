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

    UT(is_integral<char>()) == UT_TRUE;
    UT(is_integral<unsigned char>()) == UT_TRUE;
    UT(is_integral<wchar_t>()) == UT_TRUE;
    UT(is_integral<int>()) == UT_TRUE;
    UT(is_integral<unsigned>()) == UT_TRUE;
    UT(is_integral<long>()) == UT_TRUE;
    UT(is_integral<unsigned long>()) == UT_TRUE;
    UT(is_integral<long long>()) == UT_TRUE;
    UT(is_integral<unsigned long long>()) == UT_TRUE;
    UT(is_integral<float>()) == UT_FALSE;
    UT(is_integral<double>()) == UT_FALSE;

    UT(is_integral<int &>()) == UT_TRUE;
    UT(is_integral<const int>()) == UT_TRUE;
    UT(is_integral<const int &>()) == UT_TRUE;
    UT(is_integral<volatile int>()) == UT_TRUE;
    UT(is_integral<volatile int &>()) == UT_TRUE;
    UT(is_integral<const volatile int>()) == UT_TRUE;
    UT(is_integral<const int *>()) == UT_FALSE;
    UT(is_integral<remove_ptr<const int *>>()) == UT_TRUE;

    UT(is_float<float>()) == UT_TRUE;
    UT(is_float<double>()) == UT_TRUE;
    UT(is_float<long double>()) == UT_TRUE;
    UT(is_float<int>()) == UT_FALSE;

    UT(is_float<const volatile float>()) == UT_TRUE;
    UT(is_float<float *>()) == UT_FALSE;

    UT(is_numeric<int>()) == UT_TRUE;
    UT(is_numeric<float>()) == UT_TRUE;
    UT(is_numeric<bool>()) == UT_FALSE;
    UT(is_numeric<int *>()) == UT_FALSE;

    UT(is_lvalue_ref<int>()) == UT_FALSE;
    UT(is_lvalue_ref<int &>()) == UT_TRUE;
    UT(is_lvalue_ref<int &&>()) == UT_FALSE;

    UT(is_rvalue_ref<int>()) == UT_FALSE;
    UT(is_rvalue_ref<int &>()) == UT_FALSE;
    UT(is_rvalue_ref<int &&>()) == UT_TRUE;
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
    UT(hash(p)) == UT(hash(t));

    /* Retrieving components types. */
    decltype(t)::Type<0> v0 = 237;
    //decltype(t)::Type<1> v1 = "test";
    decltype(t)::Type<1> v1 = 10;
    decltype(t)::Type<2> v2 = 2.0;
    /* Retrieving components values. */
    UT(t.get<0>()) == UT(v0);
    UT(t.get<1>()) == UT(v1);
    UT(t.get<2>()) == UT(v2);

    tuple_t t2(t);
    //tuple_t t2(237, 10/* "test" */, 2.0);
    UT(len(t2)) == UT(static_cast<size_t>(3));
    UT(t2.get<0>()) == UT(v0);
    UT(t2.get<1>()) == UT(v1);
    UT(t2.get<2>()) == UT(v2);
    UT(hash(t2)) == UT(hash(t));

    t2.get<0>() = 238;
    UT(t2.get<0>()) == UT(238);
    UT(hash(t2)) != UT(hash(t));
    t2.get<0>() = v0;
    UT(hash(t2)) == UT(hash(t));

    t2.get<2>() = 3.0;
    UT(t2.get<2>()) == UT(3.0);
    UT(hash(t2)) != UT(hash(t));
    t2.get<2>() = v2;
    UT(hash(t2)) == UT(hash(t));
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

    int int_i1 = i1;
    UT(hash(int_i1)) == UT(h1);

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
