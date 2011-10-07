/*
 * /phoenix/kernel/init/init.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file init.cpp
 * Low level kernel entry point.
 *
 * This file contains the entry point for the kernel. It is called by EFI
 * loader with identity mapping active. So this file is linked to physical
 * loading address while the rest code is linked to the kernel image virtual
 * address.
 * Its task is to initialize virtual mapping - map all the kernel to new
 * virtual address, create new stack and switch to it, switch to new virtual
 * address space and call the kernel high-level entry point.
 */

#include <sys.h>
#include <boot.h>

namespace {

/* Temporal stack used for the first booting phase. */
//u8 bootStack[boot::BOOT_STACK_SIZE];

}

/** The kernel entry point. Called directly by EFI boot loader. */
extern "C" void start(boot::BootParam *bootParam);

void
start(boot::BootParam *bootParam UNUSED)
{
    cli();
    while (true) {
        pause();
    }
}
