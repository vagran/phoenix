/*
 * /phoenix/kernel/sys/cpu_caps.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file cpu_caps.h
 * Machine-independent part of definitions related to CPU capabilities
 * inquiring. The CPU capabilities enquiring by this and md_cpu_caps.h files
 * provided API should be possible from both the kernel and user space.
 */

#ifndef CPU_CAPS_H_
#define CPU_CAPS_H_

#ifdef AUTONOMOUS_LINKING
namespace {
#endif /* AUTONOMOUS_LINKING */

/** CPU-related definitions. */
namespace cpu {

/** CPU capabilities which can be inquired. */
enum CpuCapId {
    CPU_CAP_NONE,       /**< Reserved. */

    /* Paging-related capabilities. */
    CPU_CAP_PG_PGE,     /**< Global-page support. */
    CPU_CAP_PG_PAT,     /**< Page-attribute table. */
    CPU_CAP_PG_PCID,    /**< Process-context identifiers. */
    CPU_CAP_PG_SMEP,    /**< Supervisor-mode execution prevention. */
    CPU_CAP_PG_NX,      /**< Execute disable. */
    CPU_CAP_PG_1GB,     /**< 1-GByte pages. */
    CPU_CAP_PG_WIDTH_PHYS, /**< Physical address width. */
    CPU_CAP_PG_WIDTH_LIN, /**< Linear address width. */

    CPU_CAP_MAX,        /**< Number of capabilities available for inquiring. */
};

} /* namespace cpu */

#ifdef AUTONOMOUS_LINKING
}
#endif /* AUTONOMOUS_LINKING */

#include <md_cpu_caps.h>

#endif /* CPU_CAPS_H_ */
