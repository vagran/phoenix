/*
 * /phoenix/kernel/sys/vm_page.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file vm_page.h
 * Physical page representation in the kernel.
 */

#ifndef VM_PAGE_H_
#define VM_PAGE_H_

namespace vm {

/** Physical page descriptor. The kernel memory manager maintains array of
 * these descriptors which covers whole the range of managed physical memory.
 */
class Page {
public:
    /** Physical page flags. */
    enum Flags {
        /** The page is managed by the kernel memory manager. If this flag is
         * not set then the corresponding physical page is unaccessible.
         */
        F_MANAGED =         0x1,
        /** The page is a free physical memory page which can be used for the
         * OS memory allocations.
         */
        F_AVAILABLE =       0x2,
        /** ACPI data memory which can be reused by OS after ACPI is
         * initialized.
         */
        F_ACPI_RECLAIM =    0x4,
        /** ACPI non-volatile storage area. */
        F_ACPI_NVS =        0x8,
    };

    Page(long flags = 0) {
        _flags = flags;
    }

    /** Retrieve flags.
     * @return Currently set flags.
     */
    inline long GetFlags() { return _flags; }

    /** Set flags.
     *
     * @param flags New flags.
     * @return Previously set flags.
     */
    inline long SetFlags(long flags) {
        long ret = _flags;
        _flags = flags;
        return ret;
    }

    /** Get the physical address of the page which is described by this
     * descriptor.
     *
     * @return Physical address of the page.
     */
    Paddr GetPaddr();

    /** Operator @a new for the @ref Page objects. The page descriptors are
     * allocated at once in the page descriptors array. After that they are
     * constructed in place one by one using this operator.
     *
     * @param size Object size.
     * @param location Pointer to pre-allocated location of the object.
     */
    inline void *operator new(size_t size, Page *location) {
        ASSERT(size == sizeof(Page));
        return location;
    }

private:
    u32 _flags;
};

}

#endif /* VM_PAGE_H_ */
