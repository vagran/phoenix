/*
 * /phoenix/kernel/kern/debug.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file debug.cpp
 * Kernel debugging facilities implementation.
 */

#include <sys.h>

/* Kernel fatal fault handler. */
void
__Fault(const char *file, int line, const char *msg, ...)
{
    if (&LOG) {
        LOG.Alert("System fault occurred: %s:%d: ", file, line);
        va_list args;
        va_start(args, msg);
        LOG.FormatV(msg, args) << '\n';
        va_end(args);
    }

    cpu::Halt();
}
