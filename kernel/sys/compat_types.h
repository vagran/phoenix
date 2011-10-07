/*
 * /phoenix/kernel/sys/compat_types.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef COMPAT_TYPES_H_
#define COMPAT_TYPES_H_

/**
 * @file compat_types.h
 * Compatibility types definitions.
 *
 * This file defines compatibility types (e.g. POSIX types) required for
 * 3rd parties components ported to the Phoenix environment. Original Phoenix
 * code should not use them (and should not include this file) but use standard
 * types defined in @a types.h instead. Should be compilable both in C and C++.
 */

typedef i8      int8_t;
typedef i16     int16_t;
typedef i32     int32_t;
typedef i64     int64_t;
typedef u8      u_int8_t;
typedef u8      uint8_t;
typedef u16     u_int16_t;
typedef u16     uint16_t;
typedef u32     u_int32_t;
typedef u32     uint32_t;
typedef u64     u_int64_t;
typedef u64     uint64_t;

/** Timestamp compatibility type */
typedef u64     time_t;

/** File offset compatibility type */
typedef i64     off_t;

#endif /* COMPAT_TYPES_H_ */
