/*
 * /phoenix/kernel/vm/vm_page.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file vm_page.cpp
 * Implementation of kernel physical page descriptor.
 */

#include <sys.h>

using namespace vm;

Paddr
Page::GetPaddr()
{
    return mm->_physFirst + (this - mm->_pageDesc) * PAGE_SIZE;
}
