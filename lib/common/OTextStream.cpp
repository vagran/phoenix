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

using namespace text_stream;

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
    _FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (char value)
{
    _Putc(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (short value UNUSED)
{

    return *this;
}

bool
OTextStreamBase::_FormatValue(Context &ctx, bool value, char fmt UNUSED)
{
    if (ctx.Opt(O_NUM_BOOl)) {
        _Puts(ctx, value ? "1" : "0");
    } else {
        _Puts(ctx, value ? "true" : "false");
    }
    return ctx;
}
