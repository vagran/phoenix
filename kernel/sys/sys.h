/*
 * /phoenix/kernel/sys/sys.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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
#include <debug.h>

#ifndef ASSEMBLER

/* Types */
#include <types.h>
#include <compat_types.h>

/* Language support */
#include <new.h>

/* CPU specific */
#include <cpu_regs.h>
#include <cpu_instr.h>
#include <cpu_caps.h>

/* Run-time support */
#include <RetCode.h>
#include <common/gcc.h>
#include <common/stdlib.h>

#include <md_stack.h>
#include <BitString.h>
#include <lock.h>

#include <triton.h>

#include <log.h>

/* Virtual memory */
#include <vm.h>

#endif /* ASSEMBLER */

#endif /* SYS_H_ */
