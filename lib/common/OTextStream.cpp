/*
 * /phoenix/lib/common/OTextStream.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file OTextStream.cpp
 * Implementation of text output streaming.
 */

#include <sys.h>

/* Conversion context */

OTextStreamBase::Context::Context()
{
    _size = 0;
    _endOfStream = false;
    memset(_optVal, 0, sizeof(_optVal));
}

OTextStreamBase::OTextStreamBase()
{

}

bool
OTextStreamBase::_Puts(Context &ctx, const char *str)
{
    while (*str) {
        if (!_Putc(ctx, *str)) {
            break;
        }
        str++;
    }
    return ctx;
}

OTextStreamBase &
OTextStreamBase::operator << (bool value)
{
    const char *out;
    if (_globalCtx.Opt(O_NUM_BOOl)) {
        _Puts(_globalCtx, value ? "1" : "0");
    } else {
        _Puts(_globalCtx, value ? "true" : "false");
    }
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (short value UNUSED)
{

    return *this;
}

bool
OTextStreamBase::_FormatValue(size_t &size UNUSED, char fmt UNUSED, const char *opt UNUSED, size_t optSize UNUSED, bool value UNUSED)
{

    return false;
}

bool
OTextStreamBase::_FormatValue(size_t &size UNUSED, char fmt UNUSED, const char *opt UNUSED, size_t optSize UNUSED, short value UNUSED)
{
    return false;
}
