/*
 * /phoenix/kernel/sys/types.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef TYPES_H_
#define TYPES_H_

/** @file types.h
 * Common system types.
 *
 * This files contains definitions for most of commonly used system types. It
 * relies on machine-dependent types defined in file md_types.h.
 */

#include <md_types.h>

/* These types defined for compatibility with 3rd parties components */
typedef i8                  int8_t;
typedef i16                 int16_t;
typedef i32                 int32_t;
typedef i64                 int64_t;
typedef u8                  u_int8_t;
typedef u8                  uint8_t;
typedef u16                 u_int16_t;
typedef u16                 uint16_t;
typedef u32                 u_int32_t;
typedef u32                 uint32_t;
typedef u64                 u_int64_t;
typedef u64                 uint64_t;

#define U8_MAX      TYPE_UINT_MAX(u8)
#define I8_MAX      TYPE_INT_MAX(i8)
#define I8_MIN      TYPE_INT_MIN(i8)
#define UCHAR_MAX   U8_MAX
#define CHAR_MAX    I8_MAX
#define CHAR_MIN    I8_MIN

#define U16_MAX     TYPE_UINT_MAX(u16)
#define I16_MAX     TYPE_INT_MAX(i16)
#define I16_MIN     TYPE_INT_MIN(i16)
#define USHRT_MAX   U16_MAX
#define SHRT_MAX    I16_MAX
#define SHRT_MIN    I16_MIN

#define U32_MAX     TYPE_UINT_MAX(u32)
#define I32_MAX     TYPE_INT_MAX(i32)
#define I32_MIN     TYPE_INT_MIN(i32)
#define UINT_MAX    U32_MAX
#define INT_MAX     I32_MAX
#define INT_MIN     I32_MIN

#define U64_MAX     TYPE_UINT_MAX(u64)
#define I64_MAX     TYPE_INT_MAX(i64)
#define I64_MIN     TYPE_INT_MIN(i64)
#define ULONG_MAX   U64_MAX
#define LONG_MAX    I64_MAX
#define LONG_MIN    I64_MIN

#define UQUAD_MAX   ULONG_MAX
#define QUAD_MAX    LONG_MAX
#define QUAD_MIN    LONG_MIN

#define VSIZE_MAX   TYPE_UINT_MAX(vsize_t)

typedef __SIZE_TYPE__ size_t;

typedef void *Handle;

/* Variable arguments */

typedef __builtin_va_list       va_list;

#define va_start(ap, last)      __builtin_va_start((ap), (last))
#define va_arg(ap, type)        __builtin_va_arg((ap), type)
#define va_copy(dest, src)      __builtin_va_copy((dest), (src))
#define va_end(ap)              __builtin_va_end(ap)

#endif /* TYPES_H_ */
