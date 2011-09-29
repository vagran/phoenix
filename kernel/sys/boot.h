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
 * This file contains definition related to the kernel boot loading.
 */

/** The kernel gets a pointer to this structure as its entry point argument. */
struct BootParam {
    size_t size; /**< This structure full size. */
    void *efiSystemTable; /**< Pointer to the EFI system table. */ //XXX should not be void *
    /** NULL terminated string with the kernel arguments.
     * This member is a variable length array of characters.
     */
    char cmdLine[1];
};

#endif /* BOOT_H_ */
