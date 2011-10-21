/*
 * /phoenix/include/debug.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

/** @file debug.h
 * This file contains definitions for system-wide debugging and troubleshooting.
 */

#define _ASSERT(x)    do { \
    if (UNLIKELY(!(x))) { \
        FAULT("Assertion failed: '%s'", __STR(x)); \
    } \
} while (0)

/** @def ASSERT(x)
 * Macro for assertions implementations.
 * Use it to insert an assertion in the code. The assertions are compiled in
 * debug build only. So use it with caution, do not place functions calls into
 * assertion condition. Use @ref ENSURE macro for making persistent checks.
 *
 * @param x Condition to check. Issue fatal fault if evaluated to @a false.
 */
#ifdef DEBUG
#define ASSERT(x)       _ASSERT(x)
#else /* DEBUG */
#define ASSERT(x)
#endif /* DEBUG */

/** Macro for persistent assertions implementations.
 * It is always compiled in both debug and release builds so it can be used as
 * part of regular code for fatal faults checks.
 *
 * @param x Condition to check. Issue fatal fault if evaluated to @a false.
 */
#define ENSURE(x)       _ASSERT(x)

/** Macro for marking places in the code which should never be reached. Reaching
 * the marked place will cause fatal fault.
 */
#define NOT_REACHED()   FAULT("Unreachable code reached")

/** Macro for indicating a fatal fault.
 * @param msg Message to log or output.
 */
#define FAULT(msg, ...)

/** Fault handler. This function should be defined in each component to handle
 * various fatal faults, such as failed asserts, invalid internal state,
 * unexpected execution paths etc.
 *
 * @param file Name of the source file where the fault occurred.
 * @param line Line number in a source file where the fault occurred.
 * @param msg Formatted message with the description of the fault.
 */
void __Fault(const char *file, int line, const char *msg, ...)
    __FORMAT(printf, 3, 4) __NORETURN;

/** Macro for printing debug messages into debug console. Has no effect in
 * production build.
 *
 * @param msg Message to output.
 */
#ifdef DEBUG
#define TRACE(msg, ...) __Trace(__FILE__, __LINE__, msg, ## __VA_ARGS__)
#else /* DEBUG */
#define TRACE(msg, ...)
#endif /* DEBUG */

/** Debug messages handler. Should output provided message to debug console.
 *
 * @param file Name of the source file where the message created.
 * @param line Line number in a source file where the message created.
 * @param msg Formatted message to output.
 */
void __Trace(const char *file, int line, const char *msg, ...)
    __FORMAT(printf, 3, 4);

#endif /* DEBUG_H_ */
