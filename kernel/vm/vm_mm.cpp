/*
 * /phoenix/kernel/vm/vm_mm.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file vm_mm.cpp
 * Kernel virtual memory manager implementation.
 */

#include <sys.h>
#include <efi.h>

using namespace vm;

MM::MM()
{

}

void
MM::PreInitialize(vaddr_t heap UNUSED, paddr_t defaultPatRoot UNUSED, vaddr_t quickMap UNUSED,
                  void **quickMapPte UNUSED, void *memMap, size_t memMapNumDesc,
                  size_t memMapDescSize, u32 memMapDescVersion)
{

    //temp
    efi::MemoryMap mm = efi::MemoryMap(memMap,
                                       memMapNumDesc,
                                       memMapDescSize,
                                       memMapDescVersion);

    for (efi::MemoryMap::MemDesc &d: mm) {

    }
}
