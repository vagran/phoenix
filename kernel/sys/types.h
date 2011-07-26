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

/* These types should be used in all the Phoenix code */
typedef char                i8;
typedef short               i16;
typedef int                 i32;
typedef long                i64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long       u64;

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

#if (__SIZEOF_POINTER__ == 8)
typedef i64                 intptr_t;
typedef u64                 uintptr_t;
#else
typedef i32                 intptr_t;
typedef u32                 uintptr_t;
#endif

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

typedef uintptr_t   vaddr_t; /**< Virtual address */
typedef uintptr_t   vsize_t; /**< Virtual size */
typedef u64         paddr_t; /**< Physical address */
typedef u64         psize_t; /**< Physical size */

#define VSIZE_MAX   TYPE_UINT_MAX(vsize_t)

typedef __SIZE_TYPE__ size_t;

typedef void *Handle;

typedef uintptr_t   waitid_t;

typedef void (*FUNC_PTR)();

/* Variable arguments */
typedef u8* va_list;

#define va_size(arg)        roundup2(sizeof(arg), sizeof(int))
#define va_start(va, arg)   ((va) = ((u8 *)&arg) + va_size(arg))
#define va_end(va)
#define va_arg(va, type)    ((va) += va_size(type), *(type *)((va) - va_size(type)))

#endif /* TYPES_H_ */
