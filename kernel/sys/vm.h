/*
 * /phoenix/kernel/sys/vm.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef VM_H_
#define VM_H_

/** @file vm.h
 * Virtual memory machine independent definitions.
 */

#include <vm_md.h>

/** Memory page size in bytes. */
#define PAGE_SIZE       (1 << PAGE_SHIFT)

/** Convert address (either physical or virtual) to page index. */
#define Atop(x)         ((x) >> PAGE_SHIFT)
/** Convert page index to address (either physical or virtual). */
#define Ptoa(x)         ((x) << PAGE_SHIFT)

#endif /* VM_H_ */
