/*
 * /phoenix/kernel/init/init.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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

using namespace vm;
using namespace boot;

/* Local bootstrap data. */
namespace {

/* Temporal stack used for the first booting phase. */
u8 bsStack[BOOT_STACK_SIZE];

vaddr_t bsHeap; /* Current heap pointer. */
paddr_t bsDefaultLatRoot; /* Default LAT root table. */
vaddr_t bsLastMapped; /* Highest mapped virtual address. */
vaddr_t bsQuickMap; /* Quick map pages. */
void *bsQuickMapPte[NUM_QUICK_MAP]; /* Quick map page PTE. */

BootParam *bsBootParam;

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

    Vaddr va(::bsHeap);
    va.RoundUp(align);
    ::bsHeap = va + RoundUp2(size, sizeof(int));
    return va;
}

/* Memory copying function for bootstrapping stage. */
static Vaddr
BootMemcpy(Vaddr dst, Vaddr src, size_t size)
{
    u8 *p_src = src, *p_dst = dst;
    while (size) {
        *p_dst = *p_src;
        p_dst += 1;
        p_src += 1;
        size--;
    }
    return dst;
}

/* Memory filling function for bootstrapping stage. */
static Vaddr
BootMemset(Vaddr dst, u8 value, size_t size)
{
    u8 *p_dst = dst;
    while (size) {
        *p_dst = value;
        p_dst += 1;
        size--;
    }
    return dst;
}

/* Map all pages starting from kernel virtual address till current heap pointer. */
static void
MapHeap()
{
    static int quickMapIdx = 0;

    if (!bsDefaultLatRoot) {
        /* Initialize of the first call. */
        bsLastMapped = LOAD_ADDRESS;
        bsDefaultLatRoot = BootAlloc(PAGE_SIZE, PAGE_SIZE).IdentityPaddr();
        BootMemset(Paddr(bsDefaultLatRoot).IdentityVaddr(), 0, PAGE_SIZE);
        bsQuickMap = BootAlloc(NUM_QUICK_MAP * PAGE_SIZE, PAGE_SIZE);
    }

    while (bsLastMapped < bsHeap) {
        /* Map to both bootstrap and kernel VAS regions. */
        for (Vaddr va: { Vaddr(bsLastMapped), BootToMapped(bsLastMapped) }) {
            void *table = Paddr(bsDefaultLatRoot);
            for (int tableLvl = NUM_LAT_TABLES - 1; tableLvl >= 0; tableLvl--) {
                LatEntry e(va, table, tableLvl);
                Paddr pa;
                if (e.CheckFlag(LAT_EF_PRESENT)) {
                    /* Page or table is mapped, skip level. */
                    table = Paddr(e.GetAddress());
                } else if (tableLvl) {
                    /* Unmapped table, allocate and enter. */
                    pa = BootAlloc(PAGE_SIZE, PAGE_SIZE).IdentityPaddr();
                    BootMemset(pa.IdentityVaddr(), 0, PAGE_SIZE);
                    e = pa;
                    e.SetFlags(LAT_EF_PRESENT | LAT_EF_WRITE | LAT_EF_EXECUTE);
                    table = pa;
                } else {
                    /* Unmapped page, map it. */
                    e = va == bsLastMapped ?
                        va.IdentityPaddr() :
                        MappedToBoot(va).IdentityPaddr();
                    e.SetFlags(LAT_EF_PRESENT | LAT_EF_WRITE | LAT_EF_EXECUTE |
                               LAT_EF_GLOBAL);

                    if (quickMapIdx < NUM_QUICK_MAP &&
                        va == BootToMapped(bsQuickMap) + quickMapIdx * PAGE_SIZE) {

                        e.Clear();
                        bsQuickMapPte[quickMapIdx] = e;
                        quickMapIdx++;
                    }
                }
            }
        }
        bsLastMapped += PAGE_SIZE;
    }
}

static void Boot(void *arg) __NORETURN;
/* Continue booting on new temporal stack. */
static void
Boot(void *arg)
{
    BootParam *bootParam = Vaddr(arg);

    /* Heap will follow the kernel image. */
    ::bsHeap = MappedToBoot(Vaddr(&::kernEnd).RoundUp());

    /* Allocate and copy boot parameters. Pointer to the EFI system table is
     * unchanged - later it will be used as physical address.
     */
    ::bsBootParam = BootAlloc(sizeof(*bootParam));
    BootMemcpy(::bsBootParam, bootParam, sizeof(*bootParam));
    ::bsBootParam->cmdLine = BootAlloc(bootParam->cmdLineSize);
    BootMemcpy(::bsBootParam->cmdLine, bootParam->cmdLine,
               bootParam->cmdLineSize);
    ::bsBootParam->memMap = BootAlloc(bootParam->memMapDescSize *
                                      bootParam->memMapNumDesc);
    BootMemcpy(::bsBootParam->memMap, bootParam->memMap,
               bootParam->memMapDescSize * bootParam->memMapNumDesc);

    MapHeap();

    /* Tweak paging features, set new virtual address space root and turn on
     * paging if it was not yet enabled.
     */
    InitPaging(false);
    LatEntry(&bsDefaultLatRoot, NUM_LAT_TABLES).Activate();
    InitPaging(true);

    static BootstrapParam param;
    param.bootParam = ::bsBootParam;
    param.heap = ::bsHeap;
    param.defaultLatRoot = ::bsDefaultLatRoot;
    param.quickMap = ::bsQuickMap;
    BootMemcpy(param.quickMapPte, ::bsQuickMapPte, sizeof(::bsQuickMapPte));

    SwitchStack(BootToMapped(::bsStack + sizeof(::bsStack)), ::Main, &param);
}

void
Start(BootParam *bootParam)
{
    /* Disable all interrupts. */
    cpu::DisableInterrupts();

    /* Zero bootstrap BSS section. */
    BootMemset(&::kernBootBss, 0, &::kernBootEnd - &::kernBootBss);

    SwitchStack(Vaddr(::bsStack + sizeof(::bsStack)), ::Boot, bootParam);
}
