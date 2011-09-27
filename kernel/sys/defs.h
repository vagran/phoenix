/*
 * /phoenix/kernel/sys/defs.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file defs.h
 * Common system definitions.
 *
 * Here are defined common macros used across whole the system.
 */

#ifndef DEFS_H_
#define DEFS_H_

/** Get offset of member @a member in structure or class @a type. */
#define offsetof(type, member)      __builtin_offsetof (type, member)

#define __CONCAT2(x, y)             x##y
/** Macro for concatenating identifiers. */
#define __CONCAT(x, y)              __CONCAT2(x, y)

#define __STR2(x)                   # x
/** Macro for stringifying identifiers. */
#define __STR(x)                    __STR2(x)

/** Generate file-scope unique identifier with a given prefix. */
#define __UID(str)                  __CONCAT(str, __COUNTER__)

/** Provide binary constants in the code. */
#define BIN(x) ((x & 0x1) | ((x & 0x10) ? 0x2 : 0) | \
    ((x & 0x100) ? 0x4 : 0) | ((x & 0x1000) ? 0x8 : 0) | \
    ((x & 0x10000) ? 0x10 : 0) | ((x & 0x100000) ? 0x20 : 0) | \
    ((x & 0x1000000) ? 0x40 : 0) | ((x & 0x10000000) ? 0x80 : 0))

/** Number of bits in byte */
#define NBBY                        8

/** Minimal value of a given signed type. */
#define TYPE_INT_MIN(type)          ((type)1 << (sizeof(type) * NBBY - 1))
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
#define __packed                    __attribute__((packed))
#define __format(type, fmtIdx, argIdx)  __attribute__ ((format(type, fmtIdx, argIdx)))
#define __noreturn                  __attribute__ ((noreturn))
#define __noinline                  __attribute__ ((noinline))

#define Min(x, y)                   ((x) < (y) ? (x) : (y))
#define Max(x, y)                   ((x) > (y) ? (x) : (y))

#define RoundUp(size, balign)       (((size) + (balign) - 1) / (balign) * (balign))
#define RoundDown(size, balign)     ((size) / (balign) * (balign))
#define IsPowerOf2(balign)          ((((balign) - 1) & (balign)) == 0)

#define RoundUp2(size, balign)      (((size) + (balign) - 1) & (~((balign) - 1)))
#define RoundDown2(size, balign)    ((size) & (~((balign) - 1)))

/* Branching hints for the compiler */
/** Give a hint for the compiler that a given conditional statement is likely to
 * be true.
 *
 * Usage example:
 * @code
 * if (Likely(someCondition)) { ... }
 * @endcode
 */
#define Likely(condition)           __builtin_expect(!!(condition), 1)
/** Give a hint for the compiler that a given conditional statement is likely to
 * be false.
 *
 * Usage example:
 * @code
 * if (Unlikely(someCondition)) { ... }
 * @endcode
 */
#define Unlikely(condition)         __builtin_expect(!!(condition), 0)

/** Macro for marking unused parameters.
 *
 * Usage example:
 * @code
 * int SomeFunction(int UNUSED someParam) { ... }
 * @endcode
 */
#define UNUSED                      __attribute__((unused))

#endif /* DEFS_H_ */
