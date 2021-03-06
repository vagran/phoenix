/*
 * /phoenix/include/debug.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

/** @file debug.h
 * This file contains definitions for system-wide debugging and troubleshooting.
 */
namespace {}

#define _ASSERT(x)    do { \
    if (UNLIKELY(!(x))) { \
        FAULT("Assertion failed: '%s'", __STR(x)); \
    } \
} while (false)

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
#define FAULT(msg, ...) __Fault(__FILE__, __LINE__, msg, ## __VA_ARGS__)

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

#ifdef MODULE_TESTS

/** Invoke a module test.
 * @param funcName Name of the function to invoke. It is followed by its
 *      arguments. The function should return boolean value - @a true if test
 *      succeeded, @a false otherwise.
 */
#define MODULE_TEST(funcName, ...) do { \
    if (!funcName(__VA_ARGS__)) { \
        FAULT("Module test failed: " __STR(funcName)); \
    } \
} while (false)

#else /* MODULE_TESTS */

#define MODULE_TEST(funcName, ...)

#endif /* MODULE_TESTS */

#endif /* DEBUG_H_ */
