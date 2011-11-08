/*
 * /phoenix/include/defs.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file defs.h
 * Common system definitions.
 *
 * Here are defined common macros used across whole the system. Should be
 * compilable both in C and C++.
 */

#ifndef DEFS_H_
#define DEFS_H_

/** Get offset of member @a member in structure or class @a type. */
#define OFFSETOF(type, member)      __builtin_offsetof(type, member)

#define SIZEOF_ARRAY(array)         (sizeof(array) / sizeof((array)[0]))

#define __CONCAT2(x, y)             x##y
/** Macro for concatenating identifiers. */
#define __CONCAT(x, y)              __CONCAT2(x, y)

#define __STR2(x)                   # x
/** Macro for stringifying identifiers. */
#define __STR(x)                    __STR2(x)

/** Generate file-scope unique identifier with a given prefix. */
#define __UID(str)                  __CONCAT(str, __COUNTER__)

/** Give a hint for the compiler that a given conditional statement is likely to
 * be true.
 *
 * Usage example:
 * @code
 * if (LIKELY(someCondition)) { ... }
 * @endcode
 */
#define LIKELY(condition)           __builtin_expect(!!(condition), 1)
/** Give a hint for the compiler that a given conditional statement is likely to
 * be false.
 *
 * Usage example:
 * @code
 * if (UNLIKELY(someCondition)) { ... }
 * @endcode
 */
#define UNLIKELY(condition)         __builtin_expect(!!(condition), 0)

/** Macro for marking unused parameters.
 *
 * Usage example:
 * @code
 * int SomeFunction(int someParam UNUSED) { ... }
 * @endcode
 */
#define UNUSED                      __attribute__((unused))

/** Provide binary constants in the code. */
#define BIN(x) ((x & 0x1) | ((x & 0x10) ? 0x2 : 0) | \
    ((x & 0x100) ? 0x4 : 0) | ((x & 0x1000) ? 0x8 : 0) | \
    ((x & 0x10000) ? 0x10 : 0) | ((x & 0x100000) ? 0x20 : 0) | \
    ((x & 0x1000000) ? 0x40 : 0) | ((x & 0x10000000) ? 0x80 : 0))

/** Number of bits in byte */
#define NBBY                        8

/** Minimal value of a given signed type. */
#define TYPE_INT_MIN(type)          (static_cast<type>(1) << (sizeof(type) * NBBY - 1))
/** Maximal value of a given signed type. */
#define TYPE_INT_MAX(type)          (~TYPE_INT_MIN(type))
/** Maximal value of a given unsigned type. */
#define TYPE_UINT_MAX(type)         (~(type)0ul)

/** Modifier for functions which can be called from assembler code. */
#define ASMCALL                     extern "C" __attribute__((regparm(0)))

/** Macro for defining inline assembler blocks. */
#define ASM                         __asm__ __volatile__

/** Macro for explicit definition of function, method or variable assembler name. */
#define ASM_NAME(name)              __asm__(__STR2(name))

/* Shortcuts for various compiler attributes */
#define __PACKED                    __attribute__((packed))
#define __FORMAT(type, fmtIdx, argIdx)  __attribute__ ((format(type, fmtIdx, argIdx)))
#define __NORETURN                  __attribute__ ((noreturn))
#define __NOINLINE                  __attribute__ ((noinline))

/** Minimal value. */
#define MIN(x, y)                   ((x) < (y) ? (x) : (y))
/** Maximal value. */
#define MAX(x, y)                   ((x) > (y) ? (x) : (y))

/** Round up the value with specified alignment. */
#define ROUND_UP(size, align)      (((size) + (align) - 1) / (align) * (align))
/** Round down the value with specified alignment. */
#define ROUND_DOWN(size, align)    ((size) / (align) * (align))
#define IS_POWER_OF_2(value)       ((((value) - 1) & (value)) == 0)

/** Round up the value with specified alignment. Alignment must be an integer
 * power of two.
 */
#define ROUND_UP2(size, align)     (((size) + (align) - 1) & (~((align) - 1)))
/** Round down the value with specified alignment. Alignment must be an integer
 * power of two.
 */
#define ROUND_DOWN2(size, align)   ((size) & (~((align) - 1)))

//XXX should be removed after Eclipse will have 'constexpr' keyword recognized.
#define constexpr

#endif /* DEFS_H_ */
