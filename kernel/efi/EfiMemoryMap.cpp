/*
 * /phoenix/kernel/efi/EfiMemoryMap.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file EfiMemoryMap.cpp
 * EFI memory map manipulations.
 */

#include <sys.h>
#include <efi.h>

using namespace efi;

MemoryMap::MemoryMap(void *memMap, size_t numDesc, size_t descSize, u32 descVersion)
{
    _memMap = memMap;
    _numDesc = numDesc;
    _descSize = descSize;
    _descVersion = descVersion;
}

const char *
MemoryMap::GetTypeName(MemType type)
{
    static const char *names[] = {
        "Reserved",
        "Loader code",
        "Loader data",
        "Boot services code",
        "Boot services data",
        "Runtime services code",
        "Runtime services data",
        "Conventional",
        "Unusable",
        "ACPI reclaim",
        "ACPI NVS",
        "Memory mapped I/O",
        "Memory mapped I/O port space",
        "PAL code"
    };

    ASSERT(type < EfiMaxMemoryType);
    return names[type];
}

RetCode
MemoryMap::SetVirtualAddressMap()
{
    //XXX
    return RC(SUCCESS);
}
