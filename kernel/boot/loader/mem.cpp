/*
 * /phoenix/kernel/boot/loader/mem.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file mem.cpp
 * Memory management operators definition.
 */

#include <sys.h>

#include "loader.h"

void *
__operator_new(size_t size, const char *file, int line, bool isArray,
               size_t align)
{
    //XXX alignment not supported
    return LoaderAlloc(size);
}

void *
__operator_new(size_t size, bool isArray, size_t align)
{
    //XXX alignment not supported
    return LoaderAlloc(size);
}

void
__operator_delete(void *ptr, bool isArray)
{
    LoaderFree(ptr);
}
