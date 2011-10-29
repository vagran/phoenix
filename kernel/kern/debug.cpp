/*
 * /phoenix/kernel/kern/debug.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file debug.cpp
 * Kernel debugging facilities implementation.
 */

#include <sys.h>

/* Kernel fatal fault handler. */
void
__Fault(const char *file UNUSED, int line UNUSED, const char *msg UNUSED, ...)
{

    while (true) {
        cpu::pause();
    }
}
