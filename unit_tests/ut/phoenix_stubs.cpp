/*
 * /phoenix/unit_tests/ut/phoenix_stubs.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file phoenix_stubs.cpp
 * Stubs for global Phoenix functions.
 */

#include <phoenix_ut.h>

#include <sys.h>

void
__Fault(const char *file, int line, const char *msg, ...)
{
    char buf[1024];
    __ut_va_list args;
    __ut_va_start(args, msg);
    int size = ut::__ut_snprintf(buf, sizeof(buf), "Fault occurred at %s:%d: ", file, line);
    ut::__ut_vsnprintf(&buf[size], sizeof(buf) - size, msg, args);
    __ut_va_end(args);
    UT_FAIL("%s", buf);
    while (true);
}

void
__ut_trace(const char *file, int line, const char *msg, ...)
{
    __ut_va_list args;
    __ut_va_start(args, msg);
    ut::__ut_vtrace(file, line, msg, args);
    __ut_va_end(args);
}
