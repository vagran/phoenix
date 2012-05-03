/*
 * /phoenix/kernel/sys/boot.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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

#ifdef AUTONOMOUS_LINKING
namespace {
#endif /* AUTONOMOUS_LINKING */

/** This namespace contains definitions related to the system boot loading. */
namespace boot {

#endif /* __cplusplus */

enum {
    /** Size of the very initial stack region which is used on the first booting
     * phase.
     */
    BOOT_STACK_SIZE =       0x8000,
};

/** The kernel gets a pointer to this structure as its entry point argument. */
typedef struct {
    paddr_t efiSystemTable; /**< Pointer to the EFI system table. */
    u32 cmdLineSize; /**< Size of @a cmdLine in bytes. */
    char *cmdLine; /**< Null terminated string with the kernel arguments. */
    void *memMap; /**< Memory map. Describes all available memory. */
    u32 memMapNumDesc; /**< Number of descriptors in @a memMap. */
    u32 memMapDescSize; /**< One descriptor size in @a memMap. */
    u32 memMapDescVersion; /**< Descriptor version in @a memMap. */
} BootParam;

/** Boot parameters which were passed to the kernel by the boot loader are
 * stored in this variable.
 */
extern BootParam *kernBootParam;

#ifdef __cplusplus

/** This structure is passed to the higher level kernel entry point. */
struct BootstrapParam {
    BootParam *bootParam; /**< Boot parameters from the boot loader. */
    vaddr_t heap; /**< Current heap pointer. */
    paddr_t defaultLatRoot; /**< Default LAT root table. */
    vaddr_t quickMap; /**< Quick map pages. They are allocated consequentially. */
    void *quickMapPte[vm::NUM_QUICK_MAP]; /**< Quick map PTEs. */
};

/** Convert bootstrap identity mapped address to kernel virtual address. */
static inline vm::Vaddr
BootToMapped(vm::Vaddr va)
{
    return va - LOAD_ADDRESS + vm::VMA_KERNEL_TEXT;
}

/** Convert  kernel virtual address to bootstrap identity mapped address. */
static inline vm::Vaddr
MappedToBoot(vm::Vaddr va)
{
    return va - KERNEL_ADDRESS + LOAD_ADDRESS;
}

} /* namespace boot */

#ifdef AUTONOMOUS_LINKING
}
#endif /* AUTONOMOUS_LINKING */

/** Kernel higher level entry point. It never returns to caller.
 *
 * @param arg Pointer to @ref boot::BootstrapParam structure with boot parameters.
 */
void Main(void *arg) __NORETURN;

#endif /* __cplusplus */

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
