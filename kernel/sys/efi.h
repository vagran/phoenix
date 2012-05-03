/*
 * /phoenix/kernel/sys/efi.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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

/** EFI BOOLEAN type. */
typedef u8 Boolean;
/** EFI INTN type. */
typedef intptr_t Intn;
/** EFI UINTN type. */
typedef uintptr_t Uintn;
/** EFI_STATUS type. */
typedef uintptr_t EfiStatus;
/** EFI CHAR16 type. */
typedef u16 Char16;
/** EFI_HANDLE type. */
typedef void *Handle;
/** EFI_PHYSICAL_ADDRESS type. */
typedef u64 PhysAddr;
/** EFI_VIRTUAL_ADDRESS type. */
typedef u64 VirtAddr;

constexpr EfiStatus EfiErr(const int num) {
    return 0x8000000000000000 | num;
}

enum EfiStatusCode {
    EFI_SUCCESS =           0,
    EFI_LOAD_ERROR =        EfiErr(1),
    EFI_INVALID_PARAMETER = EfiErr(2),
    EFI_UNSUPPORTED =       EfiErr(3),
    EFI_BAD_BUFFER_SIZE =   EfiErr(4),
    EFI_BUFFER_TOO_SMALL =  EfiErr(5),
    EFI_NOT_READY =         EfiErr(6),
    EFI_DEVICE_ERROR =      EfiErr(7),
    EFI_WRITE_PROTECTED =   EfiErr(8),
    EFI_OUT_OF_RESOURCES =  EfiErr(9),
    EFI_VOLUME_CORRUPTED =  EfiErr(10),
    EFI_VOLUME_FULL =       EfiErr(11),
    EFI_NO_MEDIA =          EfiErr(12),
    EFI_MEDIA_CHANGED =     EfiErr(13),
    EFI_NOT_FOUND =         EfiErr(14),
    EFI_ACCESS_DENIED =     EfiErr(15),
    EFI_NO_RESPONSE =       EfiErr(16),
    EFI_NO_MAPPING =        EfiErr(17),
    EFI_TIMEOUT =           EfiErr(18),
    EFI_NOT_STARTED =       EfiErr(19),
    EFI_ALREADY_STARTED =   EfiErr(20),
    EFI_ABORTED =           EfiErr(21),
    EFI_ICMP_ERROR =        EfiErr(22),
    EFI_TFTP_ERROR =        EfiErr(23),
    EFI_PROTOCOL_ERROR =    EfiErr(24),
};

/** EFI_GUID type. */
struct Guid {
    u32 data1;
    u16 data2;
    u16 data3;
    u8 data4[8];

    //XXX initializer_list constructor
};

} /* namespace efi */

#include <md_efi.h>

namespace efi {

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
        PhysAddr paStart;
        VirtAddr vaStart;
        u64 numPages;
        u64 attr;

        /** Check if the region is available for using by an OS needs. */
        bool IsAvailable() {
            return type == EfiLoaderCode || type == EfiLoaderData ||
                   type == EfiBootServicesCode || type == EfiBootServicesData ||
                   type == EfiConventionalMemory;
        }

        /** Check if the region requires management by an OS. */
        bool NeedsManagement() {
            return !(type == EfiReservedMemoryType || type == EfiUnusableMemory ||
                     type == EfiMemoryMappedIO || type == EfiMemoryMappedIOPortSpace) ||
                   (attr & EFI_MEMORY_RUNTIME);
        }
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

    const char *GetTypeName(MemType type);

    /** Apply new virtual address map for the required areas to the firmware.
     * @return @a true if EFI call succeed, @a false otherwise.
     */
    RetCode SetVirtualAddressMap();

    /* Iterator interface */
    inline size_t size() { return _numDesc; }
    inline MemDescIterator begin() { return MemDescIterator(_memMap, _descSize, 0); }
    inline MemDescIterator end() { return MemDescIterator(_memMap, _descSize, _numDesc); }
private:
    size_t _numDesc, _descSize;
    u32 _descVersion;
    void *_memMap;
};

/** EFI_TIME type. */
struct EfiTime {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 pad1;
    u32 nanosecond;
    i16 timeZone;
    u8 daylight;
    u8 pad2;
};

/** EFI_TIME_CAPABILITIES type. */
struct EfiTimeCaps {
    u32 resolution;
    u32 accuracy;
    Boolean setsToZero;
};

/** EFI system table contains pointers to the runtime services and hardware
 * configuration tables. This class translates all runtime services calls to
 * the required EFI-specific calling convention.
 */
class SystemTable {
public:
    /** Construct object from physical pointer to the system table.
     * @param ptr Physical address of EFI system table passed to the kernel by
     *      the boot loader.
     * @param memMap EFI memory map which describes all available memory.
     * @param memMapNumDesc Number of descriptors in @a memMap.
     * @param memMapDescSize One descriptor size in @a memMap.
     * @param memMapDescVersion Descriptor version in @a memMap.
     */
    SystemTable(vm::Paddr ptr,
                void *memMap,
                size_t memMapNumDesc,
                size_t memMapDescSize,
                u32 memMapDescVersion);

    /* Run-time services. */

    /** Returns the current time and date information, and the time-keeping
     * capabilities of the hardware platform.
     */
    EfiStatus GetTime(EfiTime *time, EfiTimeCaps *caps = 0) {
        return _runtimeServices->getTime(time, caps);
    }

    // XXX add all runtime services

    EfiStatus SetVirtualAddressMap(Uintn mapSize, Uintn descSize,
                                   u32 descVersion, MemoryMap::MemDesc *virtualMap) {

        return _runtimeServices->setVirtualAddressMap(mapSize, descSize,
                                                      descVersion, virtualMap);
    }

private:
    /** EFI system table header. */
    struct TableHeader {
        u64 signature;
        u32 revision;
        u32 headerSize;
        u32 crc32;
        u32 reserved;
    };

    /** EFI system table. */
    struct Table {
        TableHeader hdr;
        Char16 *fwVendor;
        u32 fwRevision;
        Handle consoleInHandle;
        void *conIn;
        Handle consoleOutHandle;
        void *conOut;
        Handle stdErrHandle;
        void *stdErr;
        void *runtimeServices;
        void *bootServices;
        Uintn numTableEntries;
        void *configTable;
    };

    /** EFI configuration table. */
    struct ConfigTable {
        Guid vendorGuid;
        void *vendorTable;
    };

    /** EFI runtime services table. */
    struct RuntimeServicesTable {
        TableHeader hdr;
        EfiCall /* Time services. */
                getTime,
                setTime,
                getWakeupTime,
                setWakeupTime,
                /* Virtual memory services. */
                setVirtualAddressMap,
                convertPointer,
                /* Variable services. */
                getVariable,
                getNextVariableName,
                setVariable,
                /* Miscellaneous services. */
                getNextHighMonotonicCount,
                resetSystem,
                /* UEFI 2.0 capsule services. */
                updateCapsule,
                queryCapsuleCapabilities,
                /* UEFI 2.0 miscellaneous services. */
                queryVariableInfo;
    };

    enum {
        EFI_SYSTEM_TABLE_SIGNATURE = 0x5453595320494249,
        EFI_RUNTIME_SERVICES_SIGNATURE = 0x56524553544e5552,
    };

    Table *_sysTable;
    ConfigTable *_configTable;
    RuntimeServicesTable *_runtimeServices;
};

extern SystemTable *sysTable;

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
