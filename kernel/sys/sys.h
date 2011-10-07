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
#include <debug.h>

#ifndef ASSEMBLER
#include <types.h>
#include <compat_types.h>
#include <gcc.h>
#include <stdlib.h>
#include <vm.h>
#include <cpu_instr.h>
#include <md_stack.h>
#include <bitops.h>
#include <lock.h>

#endif /* ASSEMBLER */

#endif /* SYS_H_ */
