/*
 * /phoenix/include/common/stdlib.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef STDLIB_H_
#define STDLIB_H_

/** @file stdlib.h
 * Prototypes for standard library function.
 *
 * This file contains prototypes for all standard C library functions used in
 * Phoenix. Used mainly by low-level C++ support code.
 */

#include <common/crc.h>

namespace {

/** Minimal value. */
template <typename T>
inline T
Min(T x, T y) { return MIN(x, y); }

/** Maximal value. */
template <typename T>
inline T
Max(T x, T y) { return MAX(x, y); }

/** Round up the value with specified alignment. */
template <typename T, typename Tal>
inline T
RoundUp(T size, Tal align) { return ROUND_UP(size, align); }

/** Round down the value with specified alignment. */
template <typename T, typename Tal>
inline T
RoundDown(T size, Tal align) { return ROUND_DOWN(size, align); }

/** Check if specified value is an integer power of two. */
template <typename T>
inline bool
IsPowerOf2(T value) { return IS_POWER_OF_2(value); }

/** Round up the value with specified alignment. Alignment must be an integer
 * power of two.
 */
template <typename T, typename Tal>
inline T
RoundUp2(T size, Tal align)
{
    ASSERT(IsPowerOf2(align));
    return ROUND_UP2(size, align);
}

/** Round down the value with specified alignment. Alignment must be an integer
 * power of two.
 */
template <typename T, typename Tal>
inline T
RoundDown2(T size, Tal align)
{
    ASSERT(IsPowerOf2(align));
    return ROUND_DOWN2(size, align);
}

} /* Anonymous namespace */

#define memset      __builtin_memset
#define memcpy      __builtin_memcpy
#define memmove     __builtin_memmove
#define memcmp      __builtin_memcmp
#define memchr      __builtin_memchr
#define strlen      __builtin_strlen
#define strcpy      __builtin_strcpy

ASMCALL char *strncpy(char *dst, const char *src, size_t len);
ASMCALL int strcmp(const char *s1, const char *s2);
ASMCALL int strncmp(const char *s1, const char *s2, size_t len);
ASMCALL const char *strchr(const char *str, int c);
ASMCALL const char *strstr(const char *s, const char *find);

ASMCALL bool isalnum(int c);
ASMCALL bool isalpha(int c);
ASMCALL bool iscntrl(int c);
ASMCALL bool isdigit(int c);
ASMCALL bool isgraph(int c);
ASMCALL bool islower(int c);
ASMCALL bool isprint(int c);
ASMCALL bool ispunct(int c);
ASMCALL bool isspace(int c);
ASMCALL bool isupper(int c);
ASMCALL bool isxdigit(int c);
ASMCALL bool isascii(int c);
ASMCALL int tolower(int c);
ASMCALL int toupper(int c);

ASMCALL long strtol(const char *nptr, const char **endptr, unsigned base);
ASMCALL unsigned long strtoul(const char *nptr, const char **endptr, unsigned base);
ASMCALL i64 strtoq(const char *nptr, const char **endptr, unsigned base);
ASMCALL u64 strtouq(const char *nptr, const char **endptr, unsigned base);

u32 GetHash32(const char *s);
u32 GetHash32(const void *data, size_t size);

int sscanf(const char *str, const char *fmt, ...) __FORMAT(scanf, 2, 3);
int vsscanf(const char *str, char const *fmt, va_list ap) __FORMAT(scanf, 2, 0);

#include <common/OTextStream.h>
#include <common/RBTree.h>
#include <common/BuddyAllocator.h>

#endif /* STDLIB_H_ */
