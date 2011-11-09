/*
 * /phoenix/kernel/efi/EfiSystemTable.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file EfiSystemTable.cpp
 * EFI system table interface implementation.
 */

#include <sys.h>
#include <efi.h>

using namespace efi;

SystemTable *efi::sysTable;

SystemTable::SystemTable(vm::Paddr ptr,
                         void *memMap UNUSED,
                         size_t memMapNumDesc UNUSED,
                         size_t memMapDescSize UNUSED,
                         u32 memMapDescVersion UNUSED)
{
    _sysTable = vm::mm->PhysToVirt(ptr);
    if (_sysTable->hdr.signature != EFI_SYSTEM_TABLE_SIGNATURE) {
        FAULT("EFI system table signature is not recognized");
    }
    _configTable =  vm::mm->PhysToVirt(_sysTable->configTable);
    _runtimeServices = vm::mm->PhysToVirt(_sysTable->runtimeServices);
}
