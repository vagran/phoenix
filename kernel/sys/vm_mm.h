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
