/*
 * /phoenix/kernel/sys/machine/x86_64/md_types.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef MD_TYPES_H_
#define MD_TYPES_H_

/** @file md_types.h
 * Machine-dependent system types.
 *
 * This files contains definitions for basic machine-dependent system types -
 * different sizes integers, virtual and physical addresses, etc.
 */

/* These types should be used in all the Phoenix code */
typedef char                i8; /**< Signed 8 bits integer */
typedef short               i16; /**< Signed 16 bits integer */
typedef int                 i32; /**< Signed 32 bits integer */
typedef long                i64; /**< Signed 64 bits integer */
typedef unsigned char       u8; /**< Unsigned 8 bits integer */
typedef unsigned short      u16; /**< Unsigned 16 bits integer */
typedef unsigned int        u32; /**< Unsigned 32 bits integer */
typedef unsigned long       u64; /**< Unsigned 64 bits integer */

#ifndef __SIZEOF_POINTER__
#define __SIZEOF_POINTER__  sizeof(void *)
#endif

#if __SIZEOF_POINTER__ == 8
/** Integer type which can safely store a pointer */
typedef i64                 intptr_t;
/** Unsigned integer type which can safely store a pointer */
typedef u64                 uintptr_t;
#else
/** Integer type which can safely store a pointer */
typedef i32                 intptr_t;
/** Unsigned integer type which can safely store a pointer */
typedef u32                 uintptr_t;
#endif

typedef uintptr_t   vaddr_t; /**< Virtual address */
typedef uintptr_t   vsize_t; /**< Virtual size */
typedef u64         paddr_t; /**< Physical address */
typedef u64         psize_t; /**< Physical size */

#endif /* MD_TYPES_H_ */
