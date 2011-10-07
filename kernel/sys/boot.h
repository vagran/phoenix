/*
 * /phoenix/kernel/sys/boot.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef BOOT_H_
#define BOOT_H_

/** @file boot.h
 * Kernel boot loading definitions.
 *
 * This file contains definition related to the kernel boot loading. Should be
 * compilable in both C and C++ contexts.
 */

#ifdef __cplusplus
/** This namespace contains definitions related to the system boot loading. */
namespace boot {
#endif

enum {
    /** Size of the very initial stack region which is used on the first booting
     * phase.
     */
    BOOT_STACK_SIZE
};

/** The kernel gets a pointer to this structure as its entry point argument. */
typedef struct {
    void *efiSystemTable; /**< Pointer to the EFI system table. */ //XXX should not be void *
    u32 cmdLineSize; /**< Size of @a cmdLine in bytes. */
    char *cmdLine; /**< NULL terminated string with the kernel arguments. */
    void *memMap; /**< Memory map. Describes all available memory. */
    u32 memMapNumDesc; /**< Number of descriptors in @a memMap. */
    u32 memMapDescSize; /**< One descriptor size in @a memMap. */
    u32 memMapDescVersion; /**< Descriptor version in @a memMap. */
} BootParam;

#ifdef __cplusplus
} /* namespace boot */
#endif

#endif /* BOOT_H_ */