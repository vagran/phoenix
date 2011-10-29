/*
 * /phoenix/unit_tests/common/BitString/test.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <phoenix_ut.h>

#include <sys.h>

UT_TEST("Finding first set/clear bit")
{
    BitString<1024> bs;

    UT(bs.IsSet(1020)) == UT(false);
    UT(bs.IsClear(1020)) == UT(true);
    bs.Set(1020);
    UT(bs.IsSet(1020)) == UT(true);
    UT(bs.IsClear(1020)) == UT(false);
    UT(bs.FirstSet()) == UT(1020);
    bs.Invert();
    UT(bs.IsClear(1020)) == UT(true);
    UT(bs.FirstClear()) == UT(1020);
}
UT_TEST_END
