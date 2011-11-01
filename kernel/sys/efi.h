/*
 * /phoenix/kernel/sys/efi.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** Machine independent definitions for EFI (Extensible Firmware Interface)
 * functionality. Implemented according to UEFI specification rev2.3.
 */

#ifndef EFI_H_
#define EFI_H_

/** This namespace contains definitions for EFI (Extensible Firmware Interface)
 * functionality.
 */
namespace efi
{

/** EFI memory map representation which is returned by EFI @a GetMemoryMap()
 * boot service.
 */
class MemoryMap {
public:
    /** Memory regions types. */
    enum MemType {
        EfiReservedMemoryType,
        EfiLoaderCode,
        EfiLoaderData,
        EfiBootServicesCode,
        EfiBootServicesData,
        EfiRuntimeServicesCode,
        EfiRuntimeServicesData,
        EfiConventionalMemory,
        EfiUnusableMemory,
        EfiACPIReclaimMemory,
        EfiACPIMemoryNVS,
        EfiMemoryMappedIO,
        EfiMemoryMappedIOPortSpace,
        EfiPalCode,
        EfiMaxMemoryType
    };

    /** Memory regions attributes. */
    enum MemAttr {
        /** Memory cacheability attribute:  The memory region supports being
         * configured as not cacheable.
         */
        EFI_MEMORY_UC =         0x1,
        /** Memory cacheability attribute: The memory region supports being
         * configured as write combining.
         */
        EFI_MEMORY_WC =         0x2,
        /** Memory cacheability attribute:  The memory region supports being
         * configured as cacheable with a “write through” policy. Writes that
         * hit in the cache will also be written to main memory.
         */
        EFI_MEMORY_WT =         0x4,
        /** Memory cacheability attribute:  The memory region supports being
         * configured as cacheable with a “write back” policy.  Reads and writes
         * that hit in the cache do not propagate to main memory. Dirty data is
         * written back to main memory when a new cache line is allocated.
         */
        EFI_MEMORY_WB =         0x8,
        /** Memory cacheability attribute:  The memory region supports being
         * configured as not cacheable, exported, and supports the “fetch and
         * add” semaphore mechanism.
         */
        EFI_MEMORY_UCE =        0x10,
        /** Physical memory protection attribute:  The memory region supports
         * being configured as write-protected by system hardware.
         */
        EFI_MEMORY_WP =         0x1000,
        /** Physical memory protection attribute:  The memory region supports
         * being configured as read-protected by system hardware.
         */
        EFI_MEMORY_RP =         0x2000,
        /** Physical memory protection attribute:  The memory region supports
         * being configured so it is protected by system hardware from executing
         *  code.
         */
        EFI_MEMORY_XP =         0x4000,
        /** Runtime memory attribute: The memory region needs to be given a
         * virtual mapping by the operating system when @a SetVirtualAddressMap()
         * is called.
         */
        EFI_MEMORY_RUNTIME =    0x8000000000000000
    };

    /** EFI memory descriptor. */
    struct MemDesc {
        u32 type;
        u32 pad;
        paddr_t paStart;
        vaddr_t vaStart;
        u64 numPages;
        u64 attr;
    };

    /** Iterator class for memory regions descriptors iteration. */
    class MemDescIterator {
    public:
        inline MemDescIterator(void *start, size_t descSize, size_t idx) :
            _curPos(start), _descSize(descSize) {

            _curPos += descSize * idx;
        }
        inline bool operator !=(MemDescIterator &desc) { return _curPos != desc._curPos; }
        inline void operator ++() { _curPos += _descSize; }
        inline MemDesc &operator *() { return *static_cast<MemDesc *>(_curPos); }
    private:
        vm::Vaddr _curPos;
        size_t _descSize;
    };

    MemoryMap(void *memMap, size_t numDesc, size_t descSize, u32 descVersion);

    inline size_t size() { return _numDesc; }
    inline MemDescIterator begin() { return MemDescIterator(_memMap, _descSize, 0); }
    inline MemDescIterator end() { return MemDescIterator(_memMap, _descSize, _numDesc); }
private:
    size_t _numDesc, _descSize;
    u32 _descVersion;
    void *_memMap;
};

/** EFI system table contains pointers to the runtime services and hardware
 * configuration tables.
 */
class SystemTable {
public:
    /** Construct object from identity-map pointer to the system table.
     * @param ptr Identity-map pointer (treated as physical address).
     */
    SystemTable(void *ptr) { _sysTable = ptr; }

private:
    vm::Paddr _sysTable;
};

static inline MemoryMap::MemDescIterator
begin(MemoryMap &memMap)
{
    return memMap.begin();
}

static inline MemoryMap::MemDescIterator
end(MemoryMap &memMap)
{
    return memMap.end();
}

} /* namespace efi */

#endif /* EFI_H_ */
