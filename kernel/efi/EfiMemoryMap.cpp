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
