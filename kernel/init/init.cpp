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

#ifndef AUTONOMOUS_LINKING
using namespace vm;
using namespace boot;
#endif

/* Local bootstrap data. */
namespace {

/* Temporal stack used for the first booting phase. */
//u8 bootStack[boot::BOOT_STACK_SIZE];

vaddr_t bootHeap; /* Current heap pointer. */

BootParam *bootParam;

}

/** The kernel entry point. Called directly by EFI boot loader. */
extern "C" void Start(BootParam *bootParam) __NORETURN;

/* Memory allocator for bootstrapping stage. */
static Vaddr
BootAlloc(size_t size, vaddr_t align = 0)
{
    if (!align) {
        if (size >= sizeof(void *)) {
            align = sizeof(void *);
        } else {
            align = sizeof(int);
        }
    }
    ASSERT(IsPowerOf2(align));

    Vaddr va(bootHeap);
    va.RoundUp(align);
    bootHeap = va + RoundUp2(size, sizeof(int));
    return va;
}

/* Memory copying function for bootstrapping stage. */
static Vaddr
BootMemcpy(Vaddr dst, Vaddr src, size_t size)
{
    Vaddr ret = dst;
    u8 *p_src = src, *p_dst = dst;
    while (size) {
        *p_dst = *p_src;
        dst += 1;
        src += 1;
        size--;
    }
    return ret;
}

/* Memory filling function for bootstrapping stage. */
static Vaddr
BootMemset(Vaddr dst, u8 value, size_t size)
{
    Vaddr ret = dst;
    u8 *p_dst = dst;
    while (size) {
        *p_dst = value;
        dst += 1;
        size--;
    }
    return ret;
}

/* Convert bootstrap identity mapped address to kernel virtual address. */
static inline Vaddr
BootToMapped(Vaddr va)
{
    Vaddr ret = va - LOAD_ADDRESS + KERNEL_ADDRESS;
    return ret;
}

/* Convert  kernel virtual address to bootstrap identity mapped address. */
static inline Vaddr
MappedToBoot(Vaddr va)
{
    Vaddr ret = va - KERNEL_ADDRESS + LOAD_ADDRESS;
    return ret;
}

static paddr_t bsDefaultPatRoot;
static vaddr_t bsLastMapped;
static vaddr_t bsQuickMap;

static void
MapHeap()
{
    /* Map all addresses from last mapped till current heap pointer. */
    if (!bsDefaultPatRoot) {
        bsLastMapped = LOAD_ADDRESS;
        bsDefaultPatRoot = BootAlloc(PAGE_SIZE).IdentityPaddr();
        BootMemset(Paddr(bsDefaultPatRoot).IdentityVaddr(), 0, PAGE_SIZE);
        bsQuickMap = BootAlloc(PAGE_SIZE);
    }
    while (bsLastMapped < bootHeap) {
        u32 tableLvl = NUM_PAT_TABLES - 1;
        void *table = Paddr(bsDefaultPatRoot);
        do {
            PatEntry e(bsLastMapped, table, tableLvl);
            Paddr pa;
            if (e.CheckFlag(PAT_EF_PRESENT)) {
                pa = e.GetAddress();
                tableLvl--;
            }else if (tableLvl) {
                pa = BootAlloc(PAGE_SIZE).IdentityPaddr();
                e.SetAddress(pa);
                e.SetFlags(PAT_EF_PRESENT | PAT_EF_WRITE | PAT_EF_EXECUTE |
                           PAT_EF_GLOBAL);
                table = pa;
                tableLvl--;
            }
        } while (tableLvl > 0);
        bsLastMapped += PAGE_SIZE;
    }
}

void
Start(BootParam *bootParam)
{
    /* Disable all interrupts. */
    cli();

    /* Zero bootstrap BSS section. */
    BootMemset(&kernBootBss, 0, &kernBootEnd - &kernBootBss);

    /* Heap will follow the kernel image. */
    bootHeap = MappedToBoot(Vaddr(&kernEnd).RoundUp());

    ::bootParam = BootAlloc(sizeof(*bootParam));
    BootMemcpy(::bootParam, bootParam, sizeof(*bootParam));

    while (true) {
        pause();
    }
}
