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
OTextStreamBase::operator << (short value)
{
    _FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (unsigned short value)
{
    _FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (int value)
{
    _FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (unsigned int value)
{
    _FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (long value)
{
    _FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (unsigned long value)
{
    _FormatValue(_globalCtx, value);
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

bool
OTextStreamBase::_FormatInt(Context &ctx, unsigned long value, bool neg, char fmt)
{
    /* Max number conversion buffer length: a 64-bits value with radix 2. */
    char nbuf[sizeof(u64) * NBBY];

    size_t numChars = _IntToString(value, nbuf);
    while (numChars) {
        if (!_Putc(ctx, nbuf[numChars - 1])) {
            break;
        }
        numChars--;
    }
    return ctx;
}

size_t
OTextStreamBase::_IntToString(unsigned long value, char *buf, unsigned long radix,
                              bool upperCase)
{
    size_t numChars = 0;
    char const digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    ASSERT(radix >= 2 && radix <= 36);
    do {
        char c = digits[value % radix];
        buf[numChars++] = upperCase ? toupper(c) : c;
    } while (value /= radix);
    return numChars;
}
