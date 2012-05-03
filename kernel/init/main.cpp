/*
 * /phoenix/kernel/init/main.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file main.cpp
 * High-level entry point of the kernel.
 *
 * @mainpage Phoenix OS code documentation
 * @author Artyom Lebedev <artyom.lebedev@gmail.com>
 *
 * @section phoenix_overview Overview
 * Phoenix OS – object-oriented orthogonally persistent operating system. It is
 * completely written in C++ and uses OOP paradigm.
 * @par Links to the project resources:
 * http://sourceforge.net/projects/ast-phoenix/ - SourceForge project. @n
 * http://sourceforge.net/apps/trac/ast-phoenix/ - Project management front-end. @n
 * http://ast-phoenix.sourceforge.net/doc/doku.php - Project documentation in
 * Wiki format which contains a lot of information about the system concepts,
 * architecture, implementation details etc. @n
 * http://ast-phoenix.sourceforge.net/doxygen/kernel/html/ - on-line copy of the
 * code documentation.
 */

#include <sys.h>
#include <boot.h>
#include <efi.h>

boot::BootParam *boot::kernBootParam;

#ifdef MODULE_TESTS

static bool
MT_AllocOnPreinitialized()
{
    const size_t size = 10 * 1024 * 1024;
    u8 *buf = NEW u8[size];
    if (!buf) {
        return false;
    }
    memset(buf, 0x42, size);
    return true;
}

static bool
MT_AllocOnInitialized()
{
    const size_t size = 10 * 1024 * 1024;
    u8 *buf = NEW u8[size];
    if (!buf) {
        return false;
    }
    memset(buf, 0x42, size);
    return true;
}

static bool
MT_RwLocks()
{
    /* Cannot test too much on one CPU, check at least that this will not hang
     * and asserts will not fire.
     */
    RWSpinLock rwl;

    rwl.ReadLock();
    rwl.ReadLock();
    rwl.ReadUnlock();
    rwl.ReadUnlock();
    rwl.WriteLock();
    rwl.WriteUnlock();
    rwl.ReadLock();
    rwl.ReadLock();
    rwl.ReadUnlock();
    rwl.ReadUnlock();
    rwl.WriteLock();
    rwl.WriteUnlock();
    return true;
}

static bool
MT_Efi()
{
    //XXX
#if 0
    efi::EfiTime time;
    efi::EfiStatus status = efi::sysTable->GetTime(&time);
    if (status != efi::EFI_SUCCESS) {
        LOG.Debug("GetTime failed: %d", status);
        return false;
    }
    LOG.Debug("%d-%d-%d %d:%02d:%02d.%09d", time.year, time.month, time.day,
              time.hour, time.minute, time.second, time.nanosecond);
#endif
    return true;
}

#endif /* MODULE_TESTS */

void
Main(void *arg)
{
    /* Zero BSS section. */
    memset(&::kernDataEnd, 0, &::kernEnd - &::kernDataEnd);

    /* Initialize boot parameters. */
    boot::BootstrapParam *param = vm::Vaddr(arg);
    boot::kernBootParam = boot::BootToMapped(param->bootParam);
    boot::kernBootParam->cmdLine = boot::BootToMapped(boot::kernBootParam->cmdLine);
    boot::kernBootParam->memMap = boot::BootToMapped(boot::kernBootParam->memMap);

    /* Memory allocations are possible after this call. */
    vm::MM::PreInitialize(boot::BootToMapped(param->heap),
                          param->defaultLatRoot,
                          boot::BootToMapped(param->quickMap),
                          boot::BootToMapped(param->quickMapPte));

    log::InitLog();

    MODULE_TEST(MT_AllocOnPreinitialized);

    /* Finalize kernel memory management initialization. */
    vm::MM::Initialize(boot::kernBootParam->memMap,
                       boot::kernBootParam->memMapNumDesc,
                       boot::kernBootParam->memMapDescSize,
                       boot::kernBootParam->memMapDescVersion);

    /* Initialize EFI runtime support. */
    efi::sysTable = NEW_NONREC efi::SystemTable(boot::kernBootParam->efiSystemTable,
                                                boot::kernBootParam->memMap,
                                                boot::kernBootParam->memMapNumDesc,
                                                boot::kernBootParam->memMapDescSize,
                                                boot::kernBootParam->memMapDescVersion);

    MODULE_TEST(MT_AllocOnInitialized);
    MODULE_TEST(MT_RwLocks);
    MODULE_TEST(MT_Efi);

    /* Call constructors for all static objects. */
    Cxa::ConstructStaticObjects();

    NOT_REACHED();
}
