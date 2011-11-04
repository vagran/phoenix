/*
 * /phoenix/kernel/init/main.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
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

    /* Call constructors for all static objects. */
    Cxa::ConstructStaticObjects();

    /* Finalize kernel memory management initialization. */
    vm::MM::Initialize(boot::kernBootParam->memMap,
                       boot::kernBootParam->memMapNumDesc,
                       boot::kernBootParam->memMapDescSize,
                       boot::kernBootParam->memMapDescVersion);

    NOT_REACHED();
}
