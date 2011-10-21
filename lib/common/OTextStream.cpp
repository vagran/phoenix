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
    memset(_optMap, 0, sizeof(_optMap));
    memset(_optVal, 0, sizeof(_optVal));
}

OTextStreamBase::OTextStreamBase()
{

}

OTextStreamBase &
OTextStreamBase::operator << (bool value UNUSED)
{

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
