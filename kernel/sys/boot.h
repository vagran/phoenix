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
namespace
#ifndef AUTONOMOUS_LINKING
boot
#endif /* AUTONOMOUS_LINKING */
{
#endif /* __cplusplus */

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
    char *cmdLine; /**< Null terminated string with the kernel arguments. */
    void *memMap; /**< Memory map. Describes all available memory. */
    u32 memMapNumDesc; /**< Number of descriptors in @a memMap. */
    u32 memMapDescSize; /**< One descriptor size in @a memMap. */
    u32 memMapDescVersion; /**< Descriptor version in @a memMap. */
} BootParam;

extern BootParam *kernBootParam;

#ifdef __cplusplus
} /* namespace boot */
#endif

/* Symbols injected by linker script. */
extern u8   kernBootBss, /**< Start of kernel boot region BSS section. */
            kernBootEnd, /**< End of kernel boot region. */
            kernText, /**< Start of kernel main text section. */
            kernTextEnd, /**< End of kernel main text section. */
            kernRodataEnd, /**< End of kernel read-only data section. */
            kernRamdisk, /**< Start of kernel RAM disk section. */
            kernRamdiskEnd, /**< End of kernel RAM disk section. */
            kernDataEnd, /**< End of kernel data section. */
            kernEnd; /**< End of kernel loadable sections. */

/** Size of kernel RAM disk section. Emitted by linker script. */
extern u32  kernRamdiskSize;

#endif /* BOOT_H_ */
