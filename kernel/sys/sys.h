/*
 * /phoenix/kernel/sys/sys.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */


#ifndef SYS_H_
#define SYS_H_

/** @file sys.h
 * Main system include file.
 *
 * This file includes most of other required system header files. Include this
 * file in each source file.
 */

#include <defs.h>

#ifndef ASSEMBLER
#include <types.h>

/*
 * The condition in assert is not evaluated in non-DEBUG versions, so
 * be careful when using functions results in conditions.
 */
//XXX
//#define ASSERT(x)   { if (unlikely(!(x))) __assert(__FILE__, __LINE__, __STR(x)); }
#define ASSERT(x)
#ifdef DEBUG
#define assert(x)   ASSERT(x)
#else /* DEBUG */
#define assert(x)
#endif /* DEBUG */
#define ensure(x)   ASSERT(x)
extern void __assert(const char *file, u32 line, const char *cond);
#define NotReached()    panic("Unreachable code reached")
#define USED(x)     (void)(x)

#include <cpu_instr.h>
#include <bitops.h>
#include <lock.h>

#endif /* ASSEMBLER */

#endif /* SYS_H_ */
