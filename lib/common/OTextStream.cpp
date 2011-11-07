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
OTextStreamBase::operator << (const Opt &opt)
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
    FormatValue(_globalCtx, value);
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
    FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (unsigned short value)
{
    FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (int value)
{
    FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (unsigned int value)
{
    FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (long value)
{
    FormatValue(_globalCtx, value);
    return *this;
}

OTextStreamBase &
OTextStreamBase::operator << (unsigned long value)
{
    FormatValue(_globalCtx, value);
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
            ctx.SetOpt(Opt::O_LONG);
            break;
        case 'h':
        case 'H':
            ctx.SetOpt(Opt::O_SHORT);
            break;
        case '*':
            if (dotFlag) {
                ctx.SetOpt(Opt::O_PREC_REQUIRED, asteriskOrder++);
                dotFlag = false;
            } else {
                ctx.SetOpt(Opt::O_WIDTH_REQUIRED, asteriskOrder++);
            }
            break;
        case '.':
            dotFlag = true;
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
                (*fmt)--;
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

bool
OTextStreamBase::FormatV(Context &ctx, const char *fmt, va_list args)
{
    while (true) {
        char fmtChar;
        Context _ctx;

        if (!_ParseFormat(_ctx, &fmt, &fmtChar) || !fmtChar) {
            ctx += _ctx;
            return false;
        }

        if (_ctx.Opt(Opt::O_WIDTH_REQUIRED) || _ctx.Opt(Opt::O_PREC_REQUIRED)) {

            long widthOrder = 0, precOrder = 0;

            _ctx.Opt(Opt::O_WIDTH_REQUIRED, &widthOrder);
            _ctx.Opt(Opt::O_PREC_REQUIRED, &precOrder);

            if ((widthOrder < precOrder || !_ctx.Opt(Opt::O_PREC_REQUIRED)) &&
                _ctx.Opt(Opt::O_WIDTH_REQUIRED)) {

                _ctx.ClearOpt(Opt::O_WIDTH_REQUIRED);
                _ctx.SetOpt(Opt::O_WIDTH, va_arg(args, int));
            }
            if (_ctx.Opt(Opt::O_PREC_REQUIRED)) {
                _ctx.ClearOpt(Opt::O_PREC_REQUIRED);
                _ctx.SetOpt(Opt::O_PREC, va_arg(args, int));
            }
            /* Set width if it follows precision. */
            if (_ctx.Opt(Opt::O_WIDTH_REQUIRED)) {
                _ctx.ClearOpt(Opt::O_WIDTH_REQUIRED);
                _ctx.SetOpt(Opt::O_WIDTH, va_arg(args, int));
            }
        }

        switch (fmtChar) {
        case 'd':
            _ctx.SetOpt(Opt::O_SIGNED);
            /* FALL THROUGH */
        case 'o':
        case 'u':
        case 'x':
        case 'X':
            if (_ctx.Opt(Opt::O_SIGNED)) {
                if (_ctx.Opt(Opt::O_SHORT)) {
                    short x = va_arg(args, int);
                    _FormatIntValue(_ctx, x, fmtChar);
                } else if (_ctx.Opt(Opt::O_LONG)) {
                    long x = va_arg(args, long);
                    _FormatIntValue(_ctx, x, fmtChar);
                } else {
                    int x = va_arg(args, int);
                    _FormatIntValue(_ctx, x, fmtChar);
                }
            } else {
                if (_ctx.Opt(Opt::O_SHORT)) {
                    unsigned short x = va_arg(args, unsigned int);
                    _FormatIntValue(_ctx, x, fmtChar);
                } else if (_ctx.Opt(Opt::O_LONG)) {
                    unsigned long x = va_arg(args, unsigned long);
                    _FormatIntValue(_ctx, x, fmtChar);
                } else {
                    unsigned int x = va_arg(args, unsigned int);
                    _FormatIntValue(_ctx, x, fmtChar);
                }
            }
            break;
        case 'z':
            _FormatIntValue(_ctx, va_arg(args, size_t), fmtChar);
            break;
        case 's':
            FormatValue(_ctx, va_arg(args, char *), fmtChar);
            break;
        case 'c':
            {
                char c = va_arg(args, int);
                FormatValue(_ctx, c, fmtChar);
            }
            break;
        case 'p':
            FormatValue(_ctx, va_arg(args, void *), fmtChar);
            break;
        default:
            FAULT("Unknown format character specified: '%c'", fmtChar);
            break;
        }
        ctx += _ctx;
        if (!ctx) {
            return false;
        }
    }
    return ctx;
}

bool
OTextStreamBase::FormatValue(Context &ctx, bool value, char fmt UNUSED)
{
    const char *s;
    if (ctx.Opt(Opt::O_NUM_BOOL)) {
        s = value ? "1" : "0";
    } else {
        s = value ? "true" : "false";
    }

    return _FormatField(ctx, s, strlen(s));
}

bool
OTextStreamBase::FormatValue(Context &ctx, char value, char fmt UNUSED)
{
    return _FormatField(ctx, &value, 1);
}

bool
OTextStreamBase::_FormatField(Context &ctx, const char *value, size_t numChars,
                              char padChar)
{
    size_t width;
    long l;

    if (!ctx.Opt(Opt::O_WIDTH, &l) || (width = l) < numChars) {
        width = numChars;
    }

    if (!padChar) {
        if (ctx.Opt(Opt::O_PAD_CHAR, &l)) {
            padChar = l;
        } else {
            padChar = ' ';
        }
    }

    if (ctx.Opt(Opt::O_LEFT_ADJ)) {
        for (size_t i = 0; i < numChars; i++) {
            if (!_Putc(ctx, value[i])) {
                return false;
            }
        }
        for (size_t i = 0; i < width - numChars; i++) {
            if (!_Putc(ctx, padChar)) {
                return false;
            }
        }
    } else {
        for (size_t i = 0; i < width - numChars; i++) {
            if (!_Putc(ctx, padChar)) {
                return false;
            }
        }
        for (size_t i = 0; i < numChars; i++) {
            if (!_Putc(ctx, value[i])) {
                return false;
            }
        }
    }

    return ctx;
}

bool
OTextStreamBase::_FormatInt(Context &ctx, unsigned long value, bool neg, char fmt)
{
    /* Max number conversion buffer length: a 64-bits value with radix 2 plus
     * sign or blank.
     */
    char nbuf[sizeof(u64) * NBBY + 1];
    long radix, width;
    bool upperCase = false, changeWidth;

    switch (fmt) {
    case 0:
        if (!ctx.Opt(Opt::O_RADIX, &radix)) {
            radix = 10;
        } else {
            if (radix < 2 || radix > 36) {
                FAULT("Invalid radix specified (must be 2..36): %ld", radix);
                return false;
            }
        }
        break;
    case 'u':
    case 'z':
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
    case 'p':
        radix = 16;
        break;
    default:
        FAULT("Invalid format character for integer value: '%c'", fmt);
        return false;
    }

    size_t numChars = _IntToString(value, nbuf, radix, upperCase);
    size_t totalChars = numChars;

    if (ctx.Opt(Opt::O_WIDTH, &width) && !ctx.Opt(Opt::O_LEFT_ADJ) &&
        ctx.Opt(Opt::O_ZERO)) {

        changeWidth = true;
    } else {
        changeWidth = false;
    }

    if (ctx.Opt(Opt::O_SHARP)) {
        if (radix == 8) {
            nbuf[totalChars++] = '0';
        } else if (radix == 16) {
            nbuf[totalChars++] = upperCase ? 'X' : 'x';
            nbuf[totalChars++] = '0';
        }
    }

    if (ctx.Opt(Opt::O_SIGNED)) {
        if (neg) {
            nbuf[totalChars++] = '-';
        } else {
            if (ctx.Opt(Opt::O_SIGN)) {
                nbuf[totalChars++] = '+';
            } else if (ctx.Opt(Opt::O_SPACE)) {
                nbuf[totalChars++] = ' ';
            }
        }
    } else if (neg) {
        FAULT("Negative integer provided for unsigned conversion");
    }

    if (changeWidth && totalChars > numChars) {
        ctx.SetOpt(Opt::O_WIDTH, width - (totalChars - numChars));
        for (;totalChars > numChars; totalChars--) {
            if (!_Putc(ctx, nbuf[totalChars - 1])) {
                return false;
            }
        }
    } else {
        numChars = totalChars;
    }

    char padChar;
    if (ctx.Opt(Opt::O_ZERO) && !ctx.Opt(Opt::O_LEFT_ADJ)) {
        padChar = '0';
    } else {
        padChar = ' ';
    }

    /* Reverse characters in buffer. */
    for (size_t idx = 0; idx < numChars / 2; idx++) {
        char c = nbuf[idx];
        nbuf[idx] = nbuf[numChars - 1 - idx];
        nbuf[numChars - 1 - idx] = c;
    }

    return _FormatField(ctx, nbuf, numChars, padChar);
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
    if (!value) {
        return _FormatField(ctx, "(null)", sizeof("(null)") - 1);
    }
    long len;
    size_t numChars = ctx.Opt(Opt::O_PREC, &len) ? static_cast<size_t>(len) : strlen(value);
    return _FormatField(ctx, value, numChars);
}
