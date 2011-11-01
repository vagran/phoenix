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
    MM();
    /** Pre-initialize memory management. This method should be called before
     * any memory allocations can be performed in the kernel. This method
     * does not return any value. It will either succeed or cause system fault
     * in case of failure.
     *
     * @param heap Current heap pointer.
     * @param defaultPatRoot Default PAT root table.
     * @param quickMap Quick map pages. They are allocated consequentially.
     * @param quickMapPte Quick map PTEs.
     * @param memMap EFI memory map which describes all available memory.
     * @param memMapNumDesc Number of descriptors in @a memMap.
     * @param memMapDescSize One descriptor size in @a memMap.
     * @param memMapDescVersion Descriptor version in @a memMap.
     */
    static void PreInitialize(vaddr_t heap, paddr_t defaultPatRoot,
                              vaddr_t quickMap, void **quickMapPte,
                              void *memMap, size_t memMapNumDesc,
                              size_t memMapDescSize, u32 memMapDescVersion);
};

} /* namespace vm */

#endif /* VM_MM_H_ */
