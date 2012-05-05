/*
 * /phoenix/kernel/vm/vm_mm.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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
/** Temporal location for defaultLatEntryroot table (during @ref vm::MM::IS_INITIAL phase). */
static paddr_t tmpDefaultLatRoot;
/** Temporal location for last mapped heap address. Heap is mapped up to this
 * address. The value is valid during @ref vm::MM::IS_INITIAL phase.
 */
static vaddr_t tmpLastMappedHeap;

/** Map all pages starting from the last mapped heap address till the current
 * heap pointer. This function is used only during @ref vm::MM::IS_INITIAL phase. */
static void
MapHeap()
{
    QuickMap qm(tmpQuickMap, NUM_QUICK_MAP, tmpQuickMapPte);
    while (tmpLastMappedHeap < tmpHeap) {
        Vaddr va = Vaddr(tmpLastMappedHeap);
        Vaddr table = qm.Map(Paddr(tmpDefaultLatRoot));
        for (int tableLvl = NUM_LAT_TABLES - 1; tableLvl >= 0; tableLvl--) {
            LatEntry e(va, table, tableLvl);
            Paddr pa;
            if (e.CheckFlag(LAT_EF_PRESENT)) {
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
                e.SetFlags(LAT_EF_PRESENT | LAT_EF_WRITE | LAT_EF_EXECUTE);
                qm.Unmap(table);
                table = tableVa;
            } else {
                /* Unmapped page, map it. */
                e = boot::MappedToBoot(va).IdentityPaddr();
                e.SetFlags(LAT_EF_PRESENT | LAT_EF_WRITE | LAT_EF_EXECUTE |
                           LAT_EF_GLOBAL);
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
__operator_new(size_t size, const char *file, int line, bool isArray,
               size_t align)
{
#   ifdef DEBUG
        void *ptr = KmemDebugOverhead::Allocate(size, isArray, align, file, line);
#   else /* DEBUG */
        void *ptr = KmemAllocate(size, align);
#   endif /* DEBUG */
    //XXX throw exception if fails
    return ptr;
}

void *
__operator_new(size_t size, bool isArray, size_t align)
{
#   ifdef DEBUG
        void *ptr = KmemDebugOverhead::Allocate(size, isArray, align);
#   else /* DEBUG */
        void *ptr = KmemAllocate(size, align);
#   endif /* DEBUG */
    //XXX throw exception if fails
    return ptr;
}

void
__operator_delete(void *ptr, bool isArray)
{
#   ifdef DEBUG
    KmemDebugOverhead::Free(ptr, isArray);
#   else /* DEBUG */
    KmemFree(ptr);
#   endif /* DEBUG */
}

QuickMap::QuickMap(Vaddr mapBase, size_t numPages, void **mapPte) :
    _mapBase(mapBase), _numPages(numPages), _mapPte(mapPte)
{
    ASSERT(mapBase.IsAligned());
    ENSURE(numPages && numPages <= MAX_PAGES);

    for (size_t i = 0; i < numPages; i++) {
        LatEntry e(mapPte[i]);
        if (e.CheckFlag(LAT_EF_PRESENT)) {
            _mapped.Set(i);
        }
    }
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
QuickMap::Map(Paddr pa, long flags)
{
    int idx = _mapped.FirstClear();
    if (idx == -1 || idx >= static_cast<int>(_numPages)) {
        FAULT("Quick map slots exhausted");
    }
    Vaddr va = _mapBase + idx * PAGE_SIZE;
    LatEntry e(_mapPte[idx]);
    e = pa;
    e.SetFlags(flags);
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
    LatEntry e(_mapPte[idx]);
    e.Clear();
    _mapped.Clear(idx);
    InvalidateVaddr(va);
}

MM::MM(void *memMap, size_t memMapNumDesc, size_t memMapDescSize,
       u32 memMapDescVersion) :

       _quickMap(tmpQuickMap, NUM_QUICK_MAP, tmpQuickMapPte),
       _pageDesc(0),
       _defLatRoot(::tmpDefaultLatRoot)
{
    _InitializePhysMem(memMap, memMapNumDesc, memMapDescSize, memMapDescVersion);
}

void
MM::PreInitialize(vaddr_t heap, paddr_t defaultLatRoot, vaddr_t quickMap,
                  void **quickMapPte)
{
    ::tmpHeap = heap;
    ::tmpLastMappedHeap = Vaddr(heap).RoundUp();
    ::tmpQuickMap = quickMap;
    ::tmpQuickMapPte = quickMapPte;
    ::tmpDefaultLatRoot = defaultLatRoot;

    _initState = IS_PREINITIALIZED;
}

void
MM::Initialize(void *memMap, size_t memMapNumDesc, size_t memMapDescSize,
               u32 memMapDescVersion)
{
    ASSERT(!::mm);
    ::mm = NEW MM(memMap, memMapNumDesc, memMapDescSize, memMapDescVersion);
}

void
MM::_InitializePhysMem(void *memMap, size_t memMapNumDesc,
                       size_t memMapDescSize, u32 memMapDescVersion)
{
    _initState = IS_INITIALIZING;

    efi::MemoryMap map = efi::MemoryMap(memMap,
                                        memMapNumDesc,
                                        memMapDescSize,
                                        memMapDescVersion);

    /* Memory occupied by the kernel image and its initial heap. */
    _initialStart = boot::MappedToBoot(VMA_KERNEL_TEXT).IdentityPaddr();
    _initialEnd = boot::MappedToBoot(::tmpHeap).IdentityPaddr();

    /* Local allocator of physical pages. It allocates pages for LAT tables
     * when mapping PM range. The pages are taken from available physical
     * memory reported by the firmware.
     */
    class PageAllocator {
    public:
        inline PageAllocator(efi::MemoryMap &map, Paddr initialStart,
                             Paddr initialEnd) :
            _map(map), _initialStart(initialStart), _initialEnd(initialEnd) {

            _availSize = 0;
            _nextAvailSize = 0;
            _spaceAllocated = false;

            if (!_GetNextAvailable()) {
                FAULT("No available physical memory found");
            }
        }

        inline psize_t GetSize() {
            return _availSize;
        }

        /* Allocate one page. */
        Paddr AllocPage() {
            ASSERT(!_spaceAllocated);
            if (!_availSize) {
                FAULT("No more physical memory available");
            }
            Paddr pa = _avail;
            _avail += PAGE_SIZE;
            _availSize -= PAGE_SIZE;
            if (!_availSize) {
                _GetNextAvailable();
            }
            return pa;
        }

        /* Allocate space of specified size. Should be called only once. */
        Paddr AllocSpace(psize_t size) {
            ASSERT(!_spaceAllocated);
            _spaceAllocated = true;
            do {
                if (_availSize >= size) {
                    return _avail;
                }
                if (_nextAvailSize >= size) {
                    return _nextAvail;
                }
                _availSize = 0;
                _nextAvailSize = 0;
                _GetNextAvailable();
            } while (_availSize);
            return 0;
        }

        /* Get current heap pointer. */
        inline Paddr GetHeap() { return _avail; }

    private:
        efi::MemoryMap &_map;
        /* First available address. */
        Paddr _avail;
        /* Size of current available chunk. */
        psize_t _availSize;
        /* Memory occupied by the kernel image and its initial heap. */
        Paddr _initialStart, _initialEnd;

        /* Next available chunk if was split by initial area. */
        Paddr _nextAvail;
        psize_t _nextAvailSize;

        /* Indicates that AllocSpace was called so no new allocations permitted. */
        bool _spaceAllocated;

        /* Advance current available pointer to the next available area. */
        psize_t _GetNextAvailable()
        {
            if (_availSize) {
                return _availSize;
            }

            if (_nextAvailSize) {
                _avail = _nextAvail;
                _availSize = _nextAvailSize;
                _nextAvailSize = 0;
                return _availSize;
            }

            for (efi::MemoryMap::MemDesc &d: _map) {
                if (!d.IsAvailable()) {
                    continue;
                }
                if (_avail > d.paStart) {
                    /* _avail stores last used chunk. */
                    continue;
                }
                _avail = d.paStart;
                _availSize = d.numPages * PAGE_SIZE;
                _CheckInitialOverlap();
                if (_availSize) {
                    break;
                }
                if (!_availSize && _nextAvailSize) {
                    _avail = _nextAvail;
                    _availSize = _nextAvailSize;
                    _nextAvailSize = 0;
                    break;
                }
            }
            return _availSize;
        }

        /* Check if the current chunk overlaps initial memory area. Adjust the
         * chunk accordingly.
         */
        void _CheckInitialOverlap()
        {
            if (!_availSize) {
                return;
            }
            if (_avail >= _initialStart) {
                /* Possible start trimming. */
                if (_avail + _availSize <= _initialEnd) {
                    /* Fully overlapped. */
                    _availSize = 0;
                    return;
                }
                if (_avail >= _initialEnd) {
                    /* No overlapping. */
                    return;
                }
                /* Partial overlapping. */
                _availSize = _avail + _availSize - _initialEnd;
                _avail = _initialEnd;
            } else {
                /* Possible end trimming. */
                if (_avail + _availSize <= _initialStart) {
                    /* No overlapping. */
                    return;
                }
                /* Overlapping, either with splitting or without it. */
                if (_avail + _availSize > _initialEnd) {
                    /* Splitting. */
                    _nextAvail = _initialEnd;
                    _nextAvailSize = _avail + _availSize - _initialEnd;
                }
                _availSize = _initialStart - _avail;
            }
        }
    } pageAlloc(map, _initialStart, _initialEnd);

    /* Firstly find the lowest and the highest available physical addresses. */
    Paddr paMin, paMax;
    _physMemSize = 0;
    LOG.Info("System memory map:\n");
    for (efi::MemoryMap::MemDesc &d: map) {
        LOG.Format("[%016x - %016x] %s\n",
                   d.paStart, d.paStart + d.numPages * PAGE_SIZE,
                   map.GetTypeName(static_cast<efi::MemoryMap::MemType>(d.type)));

        if (!d.NeedsManagement()) {
            continue;
        }

        if (!paMax) {
            paMin = d.paStart;
            paMax = d.paStart + d.numPages * PAGE_SIZE;
        } else if (d.paStart < paMin) {
            paMin = d.paStart;
        } else if (d.paStart + d.numPages * PAGE_SIZE > paMax) {
            paMax = d.paStart + d.numPages * PAGE_SIZE;
        }

        if (d.IsAvailable()) {
            _physMemSize += d.numPages * PAGE_SIZE;
        }
    }
    _physFirst = paMin;
    _physRange = paMax - paMin;
    LOG.Info("Managed physical memory range: [%016x - %016x]", paMin, paMax);
    LOG.Info("%dMB of physical memory available", _physMemSize / (1024 * 1024));

    /* Calculate the PM mapping address. */
    cpu::CpuCaps caps;
    _physMemMap = static_cast<vaddr_t>(1) << (caps.GetCapability(cpu::CPU_CAP_PG_WIDTH_LIN) - 1);
    _physMemMap -= _physRange;

    /* Map all managed physical memory. */
    for (efi::MemoryMap::MemDesc &d: map) {
        if (!d.NeedsManagement()) {
            continue;
        }
        for (size_t pageIdx = 0; pageIdx < d.numPages; pageIdx++) {
            Paddr page = d.paStart + pageIdx * PAGE_SIZE;
            Vaddr va = PhysToVirt(page);
            Vaddr table = _quickMap.Map(Paddr(_defLatRoot));
            for (int tableLvl = NUM_LAT_TABLES - 1; tableLvl >= 0; tableLvl--) {
                LatEntry e(va, table, tableLvl);
                Paddr pa;
                if (e.CheckFlag(LAT_EF_PRESENT)) {
                    /* Page or table is mapped, skip level. */
                    pa = e.GetAddress();
                    _quickMap.Unmap(table);
                    table = _quickMap.Map(pa);
                } else if (tableLvl) {
                    /* Unmapped table, allocate and enter. */
                    pa = pageAlloc.AllocPage();
                    Vaddr tableVa = _quickMap.Map(pa);
                    memset(tableVa, 0, PAGE_SIZE);
                    e = pa;
                    e.SetFlags(LAT_EF_PRESENT | LAT_EF_WRITE | LAT_EF_EXECUTE);
                    _quickMap.Unmap(table);
                    table = tableVa;
                } else {
                    /* Unmapped page, map it. */
                    e = page;
                    e.SetFlags(LAT_EF_PRESENT | LAT_EF_WRITE | LAT_EF_EXECUTE |
                               LAT_EF_GLOBAL);
                    InvalidateVaddr(va);
                }
            }
            _quickMap.Unmap(table);
        }
    }

    /* Create page descriptors array. */
    Paddr pagesHeap = pageAlloc.GetHeap();
    size_t numPages = _physRange / PAGE_SIZE;
    Paddr pageDescPa = pageAlloc.AllocSpace(numPages * sizeof(Page));
    _pageDesc = PhysToVirt(pageDescPa);
    for (size_t i = 0; i < numPages; i++) {
        Paddr pa = _physFirst + i * PAGE_SIZE;
        long flags = 0;
        /* Exclude all occupied areas. */
        if (pa >= pagesHeap && /* LAT tables. */
            /* Page descriptors array. */
            (pa < pageDescPa || pa >= pageDescPa + numPages * PAGE_SIZE) &&
            /* Kernel initial heap. */
            (pa < _initialStart || pa >= _initialEnd)) {

            flags |= Page::F_MANAGED | Page::F_AVAILABLE;
        }
        new(&_pageDesc[i]) Page(flags);
    }

    /* Exclude all pages in reserved areas reported by the firmware. */
    for (efi::MemoryMap::MemDesc &d: map) {
        /* Re-map required areas in the firmware. */
        if (d.attr && efi::MemoryMap::EFI_MEMORY_RUNTIME) {
            d.vaStart = PhysToVirt(d.paStart);
        }

        if (d.IsAvailable()) {
            continue;
        }
        for (Paddr pa = d.paStart;
             pa < d.paStart + d.numPages * PAGE_SIZE;
             pa += PAGE_SIZE) {

            Page &page = GetPage(pa);
            long flags = page.GetFlags();
            flags &= ~Page::F_AVAILABLE;
            if (!d.NeedsManagement()) {
                flags &= ~Page::F_MANAGED;
            } else {
                if (d.type == efi::MemoryMap::EfiACPIReclaimMemory) {
                    flags |= Page::F_ACPI_RECLAIM;
                } else if (d.type == efi::MemoryMap::EfiACPIMemoryNVS) {
                    flags |= Page::F_ACPI_NVS;
                }
            }
            page.SetFlags(flags);
        }
    }

    /* Provide new virtual address map to the firmware. */
    if (NOK(map.SetVirtualAddressMap())) {
        FAULT("Failed to update the firmware virtual address map");
    }

    //XXX create allocator
    _initState = IS_PREINITIALIZED;//XXX
}
