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

namespace boot {

BootParam *kernBootParam;

}

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

    /* Call constructors for all static objects. */
    Cxa::ConstructStaticObjects();

    NOT_REACHED();
}
