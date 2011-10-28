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
OTextStreamBase::operator << (const Opt &&opt)
{
    if (opt._enable) {
        _globalCtx.SetOpt(opt._option, opt._param);
    } else {
        _globalCtx.ClearOpt(opt._option);
    }
    return *this;
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
OTextStreamBase::_ParseFormat(Context &ctx, const char **fmt, char *fmtChar)
{
    *fmtChar = 0;
    /* Skip all characters preceding format. */
    while (**fmt) {
        if (**fmt != '%') {
            if (!_Putc(ctx, **fmt)) {
                return false;
            }
        } else if ((*fmt)[1] == '%') {
            (*fmt)++;
            if (!_Putc(ctx, '%')) {
                return false;
            }
        } else {
            (*fmt)++;
            break;
        }
        (*fmt)++;
    }

    /* (*fmt) is pointing to the first character after '%' */

    bool dotFlag = 0;
    int asteriskOrder = 1;

    while (**fmt && !*fmtChar) {

        switch(**fmt) {
        case 'l':
        case 'L':
        case 'h':
        case 'H':
            break;

        case '*':
            if (dotFlag) {
                ctx.SetOpt(Opt::O_PREC_REQUIRED, asteriskOrder++);
                dotFlag = false;
            } else {
                ctx.SetOpt(Opt::O_WIDTH_REQUIRED, asteriskOrder++);
            }
            break;
        case '#':
            ctx.SetOpt(Opt::O_SHARP);
            break;
        case ' ':
            ctx.SetOpt(Opt::O_SPACE);
            break;
        case '+':
            ctx.SetOpt(Opt::O_SIGN);
            break;
        case '-':
            ctx.SetOpt(Opt::O_LEFT_ADJ);
            break;
        case '0':
            ctx.SetOpt(Opt::O_ZERO);
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                long n;
                for (n = 0; **fmt >= '0' && **fmt <= '9'; (*fmt)++) {
                    n = n * 10 + **fmt - '0';
                }
                if (dotFlag) {
                    ctx.SetOpt(Opt::O_PREC, n);
                    dotFlag = false;
                } else {
                    ctx.SetOpt(Opt::O_WIDTH, n);
                }
            }
            break;
        default:
            if (!isalpha(**fmt)) {
                FAULT("Invalid character in format specifier: '%c'", **fmt);
                return false;
            }
            /* Format character found. */
            *fmtChar = **fmt;
            ctx.SetOpt(Opt::O_FMT_PARSED, *fmtChar);
            break;
        }
        (*fmt)++;
    }

    return ctx;
}

bool
OTextStreamBase::Format(Context &ctx, const char *fmt)
{
    char fmtChar;
    long _fmtChar;
    if (ctx.Opt(Opt::O_WIDTH_REQUIRED) || ctx.Opt(Opt::O_PREC_REQUIRED)) {
        FAULT("Missing argument for width or precision");
        return false;
    }
    if (ctx.Opt(Opt::O_FMT_PARSED, &_fmtChar)) {
        FAULT("Format operator '%c' found after arguments exhausted",
              static_cast<char>(_fmtChar));
        return false;
    }
    _ParseFormat(ctx, &fmt, &fmtChar);
    if (fmtChar) {
        FAULT("Format operator '%c' found after arguments exhausted", fmtChar);
        return false;
    }
    return ctx;
}

#define CHECK_FMT_CHAR_SIGNED(type) \
    bool OTextStreamBase::_CheckFmtChar(char fmtChar, type value) \
    { \
        return fmtChar == 'd' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X'; \
    }

#define CHECK_FMT_CHAR_UNSIGNED(type) \
    bool OTextStreamBase::_CheckFmtChar(char fmtChar, unsigned type value) \
    { \
        return fmtChar == 'u' || fmtChar == 'o' || fmtChar == 'x' || fmtChar == 'X'; \
    }

CHECK_FMT_CHAR_SIGNED(short)
CHECK_FMT_CHAR_SIGNED(int)
CHECK_FMT_CHAR_SIGNED(long)
CHECK_FMT_CHAR_UNSIGNED(short)
CHECK_FMT_CHAR_UNSIGNED(int)
CHECK_FMT_CHAR_UNSIGNED(long)

bool
OTextStreamBase::_CheckFmtChar(char fmtChar, char value)
{
    return fmtChar == 'c';
}

bool
OTextStreamBase::_CheckFmtChar(char fmtChar, char *value)
{
    return fmtChar == 's';
}

bool
OTextStreamBase::_CheckFmtChar(char fmtChar, const char *value)
{
    return fmtChar == 's';
}

bool
OTextStreamBase::_FormatValue(Context &ctx, bool value, char fmt UNUSED)
{
    if (ctx.Opt(Opt::O_NUM_BOOL)) {
        _Puts(ctx, value ? "1" : "0");
    } else {
        _Puts(ctx, value ? "true" : "false");
    }
    return ctx;
}

bool
OTextStreamBase::_FormatValue(Context &ctx, char value, char fmt UNUSED)
{
    return _Putc(ctx, value);
}

bool
OTextStreamBase::_FormatValue(Context &ctx, char *value, char fmt)
{
    //XXX
    return false;
}

bool
OTextStreamBase::_FormatValue(Context &ctx, const char *value, char fmt)
{
    //XXX
    return false;
}

bool
OTextStreamBase::_FormatField(Context &ctx, const char *value, size_t numChars,
                              char padChar)
{
    long width;
    if (!ctx.Opt(Opt::O_WIDTH, &width)) {

    }
    //XXX
    return ctx;
}

bool
OTextStreamBase::_FormatInt(Context &ctx, unsigned long value, bool neg, char fmt)
{
    /* Max number conversion buffer length: a 64-bits value with radix 2 plus
     * sign or blank.
     */
    char nbuf[sizeof(u64) * NBBY + 1];
    long radix;
    bool upperCase = false;

    switch (fmt) {
    case 0:
        if (!ctx.Opt(Opt::O_RADIX, &radix)) {
            radix = 10;
        } else {
            if (radix < 2 || radix > 36) {
                FAULT("Invalid radix specified (must be 2..36): %d", radix);
                return false;
            }
        }
        break;
    case 'u':
        /* FALL THROUGH */
    case 'd':
        radix = 10;
        break;
    case 'o':
        radix = 8;
        break;
    case 'X':
        upperCase = true;
        /* FALL THROUGH */
    case 'x':
        radix = 16;
        break;
    default:
        FAULT("Invalid format character for integer value: '%c'", fmt);
        return false;
    }

    size_t numChars = _IntToString(value, nbuf, radix, upperCase);
    if (ctx.Opt(Opt::O_SHARP)) {
        if (radix == 8) {
            nbuf[numChars++] = '0';
        } else if (radix == 16) {
            nbuf[numChars++] = upperCase ? 'X' : 'x';
            nbuf[numChars++] = '0';
        }
    }

    if (ctx.Opt(Opt::O_SIGNED)) {
        if (neg) {
            nbuf[numChars++] = '-';
        } else {
            if (ctx.Opt(Opt::O_SIGN)) {
                nbuf[numChars++] = '+';
            } else if (ctx.Opt(Opt::O_SPACE)) {
                nbuf[numChars++] = ' ';
            }
        }
    } else if (neg) {
        FAULT("Negative integer provided for unsigned conversion");
    }

    /* Reverse characters in buffer. */
    for (size_t idx = 0; idx < numChars / 2; idx++) {
        char c = nbuf[idx];
        nbuf[idx] = nbuf[numChars - 1 - idx];
        nbuf[numChars - 1 - idx] = c;
    }
    return _FormatField(ctx, nbuf, numChars,
                        ctx.Opt(Opt::O_ZERO) && !ctx.Opt(Opt::O_LEFT_ADJ) ? '0' : ' ');
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

bool
OTextStreamBase::_FormatString(Context &ctx, const char *value)
{
    //XXX use _FormatField
    _Puts(ctx, value);
    return ctx;
}
