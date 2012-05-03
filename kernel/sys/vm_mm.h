/*
 * /phoenix/kernel/sys/vm_mm.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file vm_mm.h
 * Kernel virtual memory manager.
 */

#ifndef VM_MM_H_
#define VM_MM_H_

#include <vm_page.h>

namespace vm {

/** Class for creating short temporal mappings for separate pages. */
class QuickMap {
public:
    enum {
        /** Maximal number of pages which can be used for quick map. */
        MAX_PAGES = 64,
    };

    /** Construct quick map helper object.
     *
     * @param mapBase Virtual address of the first page in the VAS region
     *      reserved for quick maps. The region must be a continuous range of
     *      one or more pages.
     * @param numPages Number of pages available in provided VAS region.
     * @param mapPte Array of PTEs which correspond to virtual addresses
     *      provided in @a mapBase.
     */
    QuickMap(Vaddr mapBase, size_t numPages, void **mapPte);
    ~QuickMap();

    /** Map physical address.
     *
     * @param pa Physical address to map. One page is mapped.
     * @param flags Mapping flags for the entry, see @ref LatEntryFlags.
     * @return Virtual address of mapped page.
     */
    Vaddr Map(Paddr pa, long flags = LAT_EF_PRESENT | LAT_EF_WRITE);

    /** Unmap page which was previously mapped by @ref Map method.
     *
     * @param va Virtual address returned by @ref Map method.
     */
    void Unmap(Vaddr va);

private:
    BitString<MAX_PAGES> _mapped; /**< Mapped pages bitmap. */
    Vaddr _mapBase;
    size_t _numPages;
    void **_mapPte;
};

/** This class represents kernel virtual memory manager. */
class MM {
public:
    /** State of kernel memory management initialization. */
    enum InitState {
        /** Very initial state, nothing is initialized, memory allocations
         * not possible.
         */
        IS_INITIAL,
        /** Pre-initialized state, memory manager object is not yet created, but
         * memory allocations are already possible.
         */
        IS_PREINITIALIZED,
        /** Memory manager initialization is in progress. Memory allocations are
         * not permitted during this state.
         */
        IS_INITIALIZING,
        /** Initialized state, memory manager object is created and fully
         * functional.
         */
        IS_INITIALIZED,
    };

    /** Get current initialization state. */
    static inline InitState GetInitState() { return MM::_initState; }

    /** Pre-initialize memory management. This method should be called before
     * any memory allocations can be performed in the kernel. This method
     * does not return any value. It will either succeed or cause system fault
     * in case of failure.
     *
     * @param heap Current heap pointer.
     * @param defaultLatRoot Default LAT root table.
     * @param quickMap Quick map pages. They are allocated consequentially.
     * @param quickMapPte Quick map PTEs.
     */
    static void PreInitialize(vaddr_t heap,
                              paddr_t defaultLatRoot,
                              vaddr_t quickMap,
                              void **quickMapPte);

    /** Finalize kernel memory management initialization.
     *
     * @param memMap EFI memory map which describes all available memory.
     * @param memMapNumDesc Number of descriptors in @a memMap.
     * @param memMapDescSize One descriptor size in @a memMap.
     * @param memMapDescVersion Descriptor version in @a memMap.
     */
    static void Initialize(void *memMap,
                           size_t memMapNumDesc,
                           size_t memMapDescSize,
                           u32 memMapDescVersion);

    /** Convert physical address to corresponding virtual address in persistent
     * physical memory mapping.
     *
     * @param pa Physical address to convert.
     * @return Virtual address which can be used to access data on physical
     *      address @a pa.
     */
    inline Vaddr PhysToVirt(Paddr pa) {
        ASSERT(pa >= _physFirst && pa < _physFirst + _physRange);
        pa -= _physFirst;
        return _physMemMap + pa.IdentityVaddr();
    }

    /** Get physical page descriptor by the page physical address.
     *
     * @param pa Physical address of the page.
     * @return Reference to the physical page descriptor.
     */
    inline Page &GetPage(Paddr pa) {
        ASSERT(pa >= _physFirst && pa < _physFirst + _physRange);
        return _pageDesc[(pa - _physFirst) / PAGE_SIZE];
    }

    inline bool IsPageManaged(Paddr pa) {
        if (pa < _physFirst || pa >= _physFirst + _physRange) {
            return false;
        }
        Page &page = GetPage(pa);
        return page.GetFlags() & Page::F_MANAGED;
    }

private:
    friend class Page;

    static InitState _initState;

    /** Kernel memory manager constructor.
     *
     * @param memMap EFI memory map which describes all available memory.
     * @param memMapNumDesc Number of descriptors in @a memMap.
     * @param memMapDescSize One descriptor size in @a memMap.
     * @param memMapDescVersion Descriptor version in @a memMap.
     */
    MM(void *memMap, size_t memMapNumDesc, size_t memMapDescSize,
       u32 memMapDescVersion);

    /** Quick map for short temporal mappings. */
    QuickMap _quickMap;

    /** Persistent physical memory map start address. */
    Vaddr _physMemMap;
    /** First managed physical address. */
    Paddr _physFirst;
    /** Range of managed physical memory addresses. */
    psize_t _physRange;
    /** Array of managed physical pages descriptors. */
    Page *_pageDesc;

    /** Amount of physical memory available. */
    psize_t _physMemSize;

    /** Start address of the memory occupied by the kernel image and its initial heap. */
    Paddr _initialStart;
    /** End address of the memory occupied by the kernel image and its initial heap. */
    Paddr _initialEnd;

    /** Default LAT root table. */
    Paddr _defLatRoot;

    /** Initialize physical memory. It will create persistent PM map and page
     * descriptors array.
     *
     * @param memMap EFI memory map which describes all available memory.
     * @param memMapNumDesc Number of descriptors in @a memMap.
     * @param memMapDescSize One descriptor size in @a memMap.
     * @param memMapDescVersion Descriptor version in @a memMap.
     */
    void _InitializePhysMem(void *memMap, size_t memMapNumDesc,
                            size_t memMapDescSize, u32 memMapDescVersion);
};

/** Global memory manager singleton. */
extern MM *mm;

} /* namespace vm */

#endif /* VM_MM_H_ */
