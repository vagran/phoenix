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
#include <boot.h>
#include <efi.h>

using namespace vm;

MM *vm::mm;

VmCaps vm::vmCaps;

MM::InitState MM::_initState = MM::IS_INITIAL;

/** Temporal location for heap pointer (during @ref vm::MM::IS_INITIAL phase). */
static vaddr_t tmpHeap;
/** Temporal location for quick map pages address (during @ref vm::MM::IS_INITIAL phase). */
static vaddr_t tmpQuickMap;
/** Temporal location for quick map PTEs (during @ref vm::MM::IS_INITIAL phase). */
static void **tmpQuickMapPte;
/** Temporal location for default PAT root table (during @ref vm::MM::IS_INITIAL phase). */
static paddr_t tmpDefaultPatRoot;
/** Temporal location for last mapped heap address. Heap is mapped up to this
 * address. The value is valid during @ref vm::MM::IS_INITIAL phase.
 */
static vaddr_t tmpLastMappedHeap;

/** Map all pages starting from last mapped heap address till current heap
 * pointer. This function is used only during @ref vm::MM::IS_INITIAL phase. */
static void
MapHeap()
{
    QuickMap qm(tmpQuickMap, NUM_QUICK_MAP, tmpQuickMapPte);
    while (tmpLastMappedHeap < tmpHeap) {
        Vaddr va = Vaddr(tmpLastMappedHeap);
        Vaddr table = qm.Map(Paddr(tmpDefaultPatRoot));
        for (int tableLvl = NUM_PAT_TABLES - 1; tableLvl >= 0; tableLvl--) {
            PatEntry e(va, table, tableLvl);
            Paddr pa;
            if (e.CheckFlag(PAT_EF_PRESENT)) {
                /* Page or table is mapped, skip level. */
                pa = e.GetAddress();
                qm.Unmap(table);
                table = qm.Map(pa);
            } else if (tableLvl) {
                /* Unmapped table, allocate and enter. */
                pa = boot::MappedToBoot(Vaddr(tmpHeap).RoundUp()).IdentityPaddr();
                tmpHeap = Vaddr(tmpHeap).RoundUp() + PAGE_SIZE;
                Vaddr tableVa = qm.Map(pa);
                memset(tableVa, 0, PAGE_SIZE);
                e = pa;
                e.SetFlags(PAT_EF_PRESENT | PAT_EF_WRITE | PAT_EF_EXECUTE);
                qm.Unmap(table);
                table = tableVa;
            } else {
                /* Unmapped page, map it. */
                e = boot::MappedToBoot(va).IdentityPaddr();
                e.SetFlags(PAT_EF_PRESENT | PAT_EF_WRITE | PAT_EF_EXECUTE |
                           PAT_EF_GLOBAL);
                InvalidateVaddr(va);
            }
        }
        qm.Unmap(table);
        tmpLastMappedHeap += PAGE_SIZE;
    }
}

/** Kernel dynamic memory allocation. This function can be used only by @a new
 * operators. All the rest code must use @a new operators for all dynamic
 * memory allocations.
 *
 * @param size Size of memory chunk to allocate.
 * @param align Alignment of memory chunk. Must be power of two. Zero for
 *      default alignment.
 */
static inline void *
KmemAllocate(size_t size, size_t align = 0)
{
    Vaddr va;
    ASSERT(!align || IsPowerOf2(align));
    if (UNLIKELY(MM::GetInitState() == MM::IS_PREINITIALIZED)) {
        va = Vaddr(tmpHeap).RoundUp(align ? align : sizeof(uintptr_t));
        tmpHeap = va + size;
        MapHeap();
    } else if (LIKELY(MM::GetInitState() == MM::IS_INITIALIZED)) {

    } else {
        FAULT("Memory allocation is not permitted in current state: %d",
              MM::GetInitState());
    }
    return va;
}

/** Kernel dynamic memory freeing. This function can be used only by @a delete
 * operators. All the rest code must use @a delete operators for all dynamic
 * memory freeings.
 *
 * @param ptr Pointer to dynamically allocated memory chunk. Nothing is done if
 *      it is zero.
 */
static inline void
KmemFree(void *ptr)
{
    if (UNLIKELY(!ptr)) {
        return;
    }
}

/** Overhead for dynamically allocated memory chunks for storing debug
 * information.
 */
struct KmemDebugOverhead {
    enum {
        MAGIC = 0x766ad535,
    };

    /** Flags for memory chunks. */
    enum Flags {
        F_ARRAY =       0x1, /**< Array allocation. */
    };

    u32 magic; /**< Magic value. */
    u32 line; /**< Source file line number. */
    const char *file; /** Source file name. */
    size_t size; /**< Allocation size. */
    u32 align; /**< Allocation alignment. */
    u32 flags; /**< Memory chunk flags from @ref Flags. */

    /** Allocate chunk of memory with overhead.
     * @param size Allocation size for client data.
     * @param isArray @a true if it was array allocation.
     * @param align Client data alignment.
     * @param file Source file name.
     * @param line Source file line number.
     * @return Pointer to client data.
     */
    static void *Allocate(size_t size, bool isArray = false, size_t align = 0,
                          const char *file = 0, int line = 0);
    /** Free chunk of memory with overhead.
     *
     * @param ptr Pointer to client data.
     * @param isArray @a true if it is array allocation.
     */
    static void Free(void *ptr, bool isArray = false);
};

void *
KmemDebugOverhead::Allocate(size_t size, bool isArray, size_t align,
                            const char *file, int line)
{
    KmemDebugOverhead *oh;

    if (align) {
        /* Client data must be properly aligned. */
        size_t numOhBlocks = sizeof(KmemDebugOverhead) / align + 1;
        vm::Vaddr mem = KmemAllocate(size + align * numOhBlocks);
        mem += align * numOhBlocks - sizeof(KmemDebugOverhead);
        oh = mem;
    } else {
        oh = vm::Vaddr(KmemAllocate(size + sizeof(KmemDebugOverhead)));
    }
    if (!oh) {
        return 0;
    }

    oh->magic = MAGIC;
    oh->size = size;
    oh->align = align;
    oh->file = file;
    oh->line = line;
    oh->flags = 0;
    if (isArray) {
        oh->flags |= F_ARRAY;
    }

    return oh + 1;
}

void
KmemDebugOverhead::Free(void *ptr, bool isArray)
{
    KmemDebugOverhead *oh = static_cast<KmemDebugOverhead *>(ptr) - 1;

    ENSURE(oh->magic == MAGIC);
    if (isArray != !!(oh->flags & F_ARRAY)) {
        FAULT("Array and non-array allocation and freeing mixed.");
    }

    if (oh->align) {
        size_t numOhBlocks = sizeof(KmemDebugOverhead) / oh->align + 1;
        Vaddr mem = ptr;
        mem -= oh->align * numOhBlocks;
        KmemFree(mem);
    } else {
        KmemFree(oh);
    }
}

void *
operator new(size_t size)
{
#ifdef DEBUG
    return KmemDebugOverhead::Allocate(size);
#else /* DEBUG */
    return KmemAllocate(size);
#endif /* DEBUG */
}

void *
operator new[](size_t size)
{
#ifdef DEBUG
    return KmemDebugOverhead::Allocate(size, true);
#else /* DEBUG */
    return KmemAllocate(size);
#endif /* DEBUG */
}

void *
operator new(size_t size, size_t align, bool nonRec)
{
#ifdef DEBUG
    void *ptr = KmemDebugOverhead::Allocate(size, false, align);
#else /* DEBUG */
    void *ptr = KmemAllocate(size, align);
#endif /* DEBUG */

    if (UNLIKELY(!ptr && nonRec)) {
        FAULT("Non-recoverable memory allocation failed");
    }
    return ptr;
}

void *
operator new[](size_t size, size_t align, bool nonRec)
{
#ifdef DEBUG
    void *ptr = KmemDebugOverhead::Allocate(size, true, align);
#else /* DEBUG */
    void *ptr = KmemAllocate(size, align);
#endif /* DEBUG */

    if (UNLIKELY(!ptr && nonRec)) {
        FAULT("Non-recoverable memory allocation failed");
    }
    return ptr;
}

void *
operator new(size_t size, const char *file, int line, size_t align, bool nonRec)
{
#ifdef DEBUG
    void *ptr = KmemDebugOverhead::Allocate(size, false, align, file, line);
#else /* DEBUG */
    void *ptr = KmemAllocate(size, align);
#endif /* DEBUG */

    if (!ptr && nonRec) {
        FAULT("Non-recoverable memory allocation failed from '%s':%d",
              file, line);
    }
    return ptr;
}

void *
operator new[](size_t size, const char *file, int line, size_t align, bool nonRec)
{
#ifdef DEBUG
    void *ptr = KmemDebugOverhead::Allocate(size, true, align, file, line);
#else /* DEBUG */
    void *ptr = KmemAllocate(size, align);
#endif /* DEBUG */

    if (!ptr && nonRec) {
        FAULT("Non-recoverable memory allocation failed from '%s':%d",
              file, line);
    }
    return ptr;
}

void
operator delete(void *ptr)
{
#ifdef DEBUG
    KmemDebugOverhead::Free(ptr);
#else /* DEBUG */
    KmemFree(ptr);
#endif /* DEBUG */
}

void
operator delete[](void *ptr)
{
#ifdef DEBUG
    KmemDebugOverhead::Free(ptr, true);
#else /* DEBUG */
    KmemFree(ptr);
#endif /* DEBUG */
}

QuickMap::QuickMap(Vaddr mapBase, size_t numPages, void **mapPte) :
    _mapBase(mapBase), _numPages(numPages), _mapPte(mapPte)
{
    ASSERT(mapBase.IsAligned());
    ENSURE(numPages && numPages <= MAX_PAGES);
}

QuickMap::~QuickMap()
{
    for (size_t idx = 0; idx < _numPages; idx++) {
        if (_mapped[idx]) {
            Unmap(_mapBase + PAGE_SIZE * idx);
        }
    }
}

Vaddr
QuickMap::Map(Paddr pa)
{
    int idx = _mapped.FirstClear();
    if (idx == -1 || idx >= static_cast<int>(_numPages)) {
        FAULT("Quick map slots exhausted");
    }
    Vaddr va = _mapBase + idx * PAGE_SIZE;
    PatEntry e(_mapPte[idx]);
    e = pa;
    e.SetFlags(PAT_EF_PRESENT | PAT_EF_WRITE | PAT_EF_EXECUTE);
    InvalidateVaddr(va);
    _mapped.Set(idx);
    return va;
}

void
QuickMap::Unmap(Vaddr va)
{
    ASSERT(va.IsAligned());
    ASSERT(va >= _mapBase && va < _mapBase + _numPages * PAGE_SIZE);
    size_t idx = (va - _mapBase) / PAGE_SIZE;
    ASSERT(_mapped[idx]);
    PatEntry e(_mapPte[idx]);
    e.Clear();
    _mapped.Clear(idx);
    InvalidateVaddr(va);
}

MM::MM(void *memMap, size_t memMapNumDesc, size_t memMapDescSize,
       u32 memMapDescVersion)
{
    //temp
    efi::MemoryMap mm = efi::MemoryMap(memMap,
                                       memMapNumDesc,
                                       memMapDescSize,
                                       memMapDescVersion);

    for (efi::MemoryMap::MemDesc &d: mm) {
        (void)d;
    }
}

void
MM::PreInitialize(vaddr_t heap, paddr_t defaultPatRoot, vaddr_t quickMap,
                  void **quickMapPte)
{
    ::tmpHeap = heap;
    ::tmpLastMappedHeap = Vaddr(heap).RoundUp();
    ::tmpQuickMap = quickMap;
    ::tmpQuickMapPte = quickMapPte;
    ::tmpDefaultPatRoot = defaultPatRoot;

    _initState = IS_PREINITIALIZED;
}

void
MM::Initialize(void *memMap, size_t memMapNumDesc, size_t memMapDescSize,
               u32 memMapDescVersion)
{
    ::mm = NEW_NONREC MM(memMap, memMapNumDesc, memMapDescSize, memMapDescVersion);
}
