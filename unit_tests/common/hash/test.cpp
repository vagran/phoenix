/*
 * /phoenix/unit_tests/common/hash/test.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file test.cpp
 * Unit tests for Phoenix lookup hash algorithm.
 */

#include <phoenix_ut.h>

#include <sys.h>

UT_TEST("Hash class")
{
    static const char data[] =
        "012345678abcdef012345678abcdef012345678abcdef012345678abcdef"
        "012345678abcdef012345678abcdef012345678abcdef012345678abcdef"
        "012345678abcdef012345678abcdef012345678abcdef012345678abcdef"
        "012345678abcdef012345678abcdef012345678abcdef012345678abcdef";
    Hash h1;
    h1.Feed(&data[0], 64);
    Hash h2;
    h2.Feed(&data[0], 128);
    UT(h1.Get64()) != UT(h2.Get64());
    h2.Reset();
    h2.Feed(&data[0], 64);
    UT(h1.Get64()) == UT(h2.Get64());
    h2.Reset();
    h2.Feed(&data[0], 1);
    h2.Feed(&data[1], 1);
    h2.Feed(&data[2], 33);
    UT(h2.Get32()) == UT(h2.Get32());
    UT(h2.Get64()) == UT(h2.Get64());
    h2.Feed(&data[35], 21);
    h2.Feed(&data[56], 8);
    UT(h2.GetLength()) == UT_SIZE(64);
    UT(h1.Get64()) == UT(h2.Get64());
    UT(static_cast<u64>(h1)) == UT(static_cast<u64>(h2));
    UT(static_cast<u32>(h1)) == UT(static_cast<u32>(h2));
}
UT_TEST_END
