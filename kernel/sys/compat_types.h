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
 * 3rd parties components ported to the Phoenix environment. Should be
 * compilable both in C and C++.
 */

/** Timestamp compatibility type */
typedef u64     time_t;

/** File offset compatibility type */
typedef i64     off_t;

#endif /* COMPAT_TYPES_H_ */
