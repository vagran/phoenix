/*
 * /phoenix/kernel/sys/vm_mm.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file vm_mm.h
 * Kernel virtual memory manager.
 */

#ifndef VM_MM_H_
#define VM_MM_H_

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
     * @return Virtual address of mapped page.
     */
    Vaddr Map(Paddr pa);

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
     * @param defaultPatRoot Default PAT root table.
     * @param quickMap Quick map pages. They are allocated consequentially.
     * @param quickMapPte Quick map PTEs.
     */
    static void PreInitialize(vaddr_t heap,
                              paddr_t defaultPatRoot,
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

private:
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

    /** Current heap pointer. */
    Vaddr _heap;

    /** Quick map first page virtual address. */
    Vaddr _quickMap;
    /** PTEs for quick map pages. */
    PatEntry _quickMapPte[NUM_QUICK_MAP];
};

/** Global memory manager singleton. */
extern MM *mm;

} /* namespace vm */

#endif /* VM_MM_H_ */
