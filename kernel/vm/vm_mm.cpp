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

MM *vm::mm;

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
    ASSERT(!align || IsPowerOf2(align));
    if (UNLIKELY(MM::GetInitState() == MM::IS_PREINITIALIZED)) {

    } else if (LIKELY(MM::GetInitState() == MM::IS_INITIALIZED)) {

    } else {
        FAULT("Memory allocation is not permitted in current state: %d",
              MM::GetInitState());
    }
    return 0;
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
MM::PreInitialize(vaddr_t heap UNUSED, paddr_t defaultPatRoot UNUSED, vaddr_t quickMap UNUSED,
                  void **quickMapPte UNUSED)
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
