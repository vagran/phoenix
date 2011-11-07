/*
 * /phoenix/kernel/sys/arch/x86_64/vm_md.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef MD_VM_H_
#define MD_VM_H_

/** @file md_vm.h
 * Virtual memory machine dependent definitions.
 */

namespace vm {

enum {
    /** Number of bits to shift to get a memory page frame. */
    PAGE_SHIFT =            12,

    /** Number of linear address translation tables in the hierarchy. */
    NUM_LAT_TABLES =        4,
};

/** Memory page index. */
typedef u64 PageIdx;

/** Index of an entry in a linear address translation table. */
typedef u32 LatEntryIdx;

/** Process context identifier. */
typedef u32 ProcCtxId;

/** Helper class to extract components (LAT tables entries indices and offset
 * in a page) from a virtual address. Defines the layout of LAT tables
 * hierarchy and virtual address fields layout.
 */
class VaddrDecoder {
public:
    /** Construct decoder object from virtual address. */
    inline VaddrDecoder(vaddr_t va = 0) { _va.va = va; }

    /** Get size of specifiedLatEntrytable.
     *
     * @param tableLvl Null-based table level starting from least significant
     *      (e.g. page table, page directory, ...).
     * @return Number of entries in a specified table.
     */
    static inline u32 GetTableSize(u32 UNUSED tableLvl) {
        ASSERT(tableLvl < NUM_LAT_TABLES);
        /* 512 entries in eachLatEntrytable. */
        return 512;
    }

    /** Get entry index in a specifiedLatEntrytable of the given virtual address.
     *
     * @param tableLvl LAT table level.
     * @return Index of the entry in the specifiedLatEntrytable corresponding to
     *      the given virtual address.
     */
    inline LatEntryIdx GetEntryIndex(u32 tableLvl) {
        switch (tableLvl) {
        case 0:
            return _va.fields.tblIdx0;
        case 1:
            return _va.fields.tblIdx1;
        case 2:
            return _va.fields.tblIdx2;
        case 3:
            return _va.fields.tblIdx3;
        }
        FAULT("Table index is out of range: %d", tableLvl);
        return 0;
    }

    /** Get offset in a page for a given virtual address. */
    inline vaddr_t GetPageOffset() {
        return _va.fields.pageOffset;
    }

private:
    struct VaFields {
        vaddr_t     pageOffset:PAGE_SHIFT,
                    tblIdx0:9, /* Page table */
                    tblIdx1:9, /* Page directory */
                    tblIdx2:9, /* Page directory pointers table */
                    tblIdx3:9, /* PML4 */
                    :16;
    };

    volatile union {
        vaddr_t va;
        VaFields fields;
    } _va;
};

/** Class representing LAT table entry. */
class LatEntry {
public:
    inline LatEntry() {
        _ptr.raw = 0;
        _tableLvl = 0;
    }

    /** Create entry by a table and virtual address. The entry in the table
     * selected accordingly to virtual address provided.
     * @param va Virtual address.
     * @param table Pointer to the table.
     * @param tableLvl Null-based table level starting from least significant
     *      (e.g. page table, page directory, ...).
     */
    inline LatEntry(vaddr_t va, void *table, u32 tableLvl = 0) {
        Set(va, table, tableLvl);
    }

    /** Create entry by direct pointer.
     *
     * @param entry Pointer to the entry.
     * @param tableLvl Level of the table which contains the entry.
     */
    inline LatEntry(void *entry, u32 tableLvl = 0) {
        Set(entry, tableLvl);
    }

    /** Set entry by a table and virtual address. The entry in the table
     * selected accordingly to virtual address provided.
     * @param va Virtual address.
     * @param table Pointer to the table.
     * @param tableLvl Null-based table level starting from least significant
     *      (e.g. page table, page directory, ...).
     */
    inline void Set(vaddr_t va, void *table, u32 tableLvl = 0) {
        ASSERT(tableLvl <= NUM_LAT_TABLES);
        _tableLvl = tableLvl;
        _ptr.ptr = table;
        VaddrDecoder dec(va);
        _ptr.raw += dec.GetEntryIndex(tableLvl);
    }

    /** Set entry by direct pointer.
     *
     * @param entry Pointer to the entry.
     * @param tableLvl Level of the table which contains the entry.
     */
    inline void Set(void *entry, u32 tableLvl = 0) {
        ASSERT(tableLvl <= NUM_LAT_TABLES);
        _tableLvl = tableLvl;
        _ptr.ptr = entry;
    }

    /** Check if the page mapped by the entry was accessed since last flag
     * reset.
     * @return @a true if the page was accessed.
     */
    inline bool IsAccessed() {
        if (_tableLvl == NUM_LAT_TABLES) {
            /* CR3 does not have the flag. */
            return false;
        }
        return _ptr.entryPage->accessed;
    }

    /** Set or clear @a accessed flag for an entry.
     * @param flag Value to set in the flag.
     *
     * @return Previous value of the flag.
     */
    inline bool SetAccessed(bool flag = true) {
        if (_tableLvl == NUM_LAT_TABLES) {
            /* CR3 does not have the flag. */
            return false;
        }
        bool curFlag = _ptr.entryPage->accessed;
        _ptr.entryPage->accessed = flag;
        return curFlag;
    }

    /** Check if the page mapped by the entry was modified since last flag
     * reset.
     * @return @a true if the page was modified.
     */
    inline bool IsDirty() {
        if (_tableLvl == NUM_LAT_TABLES) {
            /* CR3 does not have the flag. */
            return false;
        }
        return _ptr.entryPage->dirty;
    }

    /** Set or clear @a dirty flag for an entry.
     * @param flag Value to set in the flag.
     *
     * @return Previous value of the flag.
     */
    inline bool SetDirty(bool flag = true) {
        if (_tableLvl == NUM_LAT_TABLES) {
            /* CR3 does not have the flag. */
            return false;
        }
        bool curFlag = _ptr.entryPage->dirty;
        _ptr.entryPage->dirty = flag;
        return curFlag;
    }

    /** Check specified flag inLatEntryentry.
     * @param flag Flag to check.
     * @return Specified flag value.
     */
    bool CheckFlag(LatEntryFlags flag) {
        ASSERT(_tableLvl <= NUM_LAT_TABLES);
        if (_tableLvl == NUM_LAT_TABLES) {
            switch (flag) {
            case LAT_EF_WRITE_THROUGH:
                return _ptr.cr3->pwt;
            case LAT_EF_CACHE_DISABLE:
                return _ptr.cr3->pcd;
            default:
                return false;
            }
        }
        switch (flag) {
        case LAT_EF_PRESENT:
            return _ptr.entryPage->present;
        case LAT_EF_WRITE:
            return _ptr.entryPage->write;
        case LAT_EF_USER:
            return _ptr.entryPage->user;
        case LAT_EF_WRITE_THROUGH:
            return _ptr.entryPage->writeThrough;
        case LAT_EF_CACHE_DISABLE:
            return _ptr.entryPage->cacheDisable;
        case LAT_EF_EXECUTE:
            return !_ptr.entryPage->executeDisable;
        case LAT_EF_GLOBAL:
            return _ptr.entryPage->global;
        }
        FAULT("Invalid flag specified: %d", flag);
        return false;
    }

    /** Set or clear specified flag inLatEntryentry.
     *
     * @param flag Flag to set or clear. Corresponding machine-dependent flag
     *      will be affected.
     * @param setIt Set the flag if @a true, clear otherwise.
     * @return Previous flag state.
     */
    bool SetFlag(LatEntryFlags flag, bool setIt = true) {
        bool prev;
        ASSERT(_tableLvl <= NUM_LAT_TABLES);
        if (_tableLvl == NUM_LAT_TABLES) {
            switch (flag) {
            case LAT_EF_WRITE_THROUGH:
                prev = _ptr.cr3->pwt;
                _ptr.cr3->pwt = setIt ? 1 : 0;
                break;
            case LAT_EF_CACHE_DISABLE:
                prev = _ptr.cr3->pcd;
                _ptr.cr3->pcd = setIt ? 1 : 0;
                break;
            default:
                prev = false;
                break;
            }
            return prev;
        }
        switch (flag) {
        case LAT_EF_PRESENT:
            prev = _ptr.entryPage->present;
            _ptr.entryPage->present = setIt ? 1 : 0;
            break;
        case LAT_EF_WRITE:
            prev = _ptr.entryPage->write;
            _ptr.entryPage->write = setIt ? 1 : 0;
            break;
        case LAT_EF_USER:
            prev = _ptr.entryPage->user;
            _ptr.entryPage->user = setIt ? 1 : 0;
            break;
        case LAT_EF_WRITE_THROUGH:
            prev = _ptr.entryPage->writeThrough;
            _ptr.entryPage->writeThrough = setIt ? 1 : 0;
            break;
        case LAT_EF_CACHE_DISABLE:
            prev = _ptr.entryPage->cacheDisable;
            _ptr.entryPage->cacheDisable = setIt ? 1 : 0;
            break;
        case LAT_EF_EXECUTE:
            prev = !_ptr.entryPage->executeDisable;
            if (vmCaps.IsValid() && vmCaps.nx) {
                _ptr.entryPage->executeDisable = setIt ? 0 : 1;
            }
            break;
        case LAT_EF_GLOBAL:
            prev = _ptr.entryPage->global;
            if (_tableLvl == 0 && vmCaps.IsValid() && vmCaps.pge) {
                _ptr.entryPage->global = setIt ? 1 : 0;
            }
            break;
        default:
            FAULT("Invalid flag specified: %d", flag);
            break;
        }
        return prev;
    }

    /** SetLatEntryentry flags.
     *
     * @param flags Flags which must be set. Any number of flags defined by
     *      @ref LatEntryFlags can be specified. Corresponding machine-dependent
     *      flags will be set, flags which were not specified will be cleared.
     * @return Previous combination of the flags.
     */
    long SetFlags(long flags) {
        static const LatEntryFlags allFlags[] = {
            LAT_EF_PRESENT,
            LAT_EF_WRITE,
            LAT_EF_USER,
            LAT_EF_WRITE_THROUGH,
            LAT_EF_CACHE_DISABLE,
            LAT_EF_EXECUTE,
            LAT_EF_GLOBAL
        };
        long prev;

        for (auto flag: allFlags) {
            if (SetFlag(flag, flags & flag)) {
                prev |= flag;
            }
        }
        return prev;
    }

    /** Get physical address pointed by the entry. */
    inline paddr_t GetAddress() {
        return _ptr.entryPage->pa << PAGE_SHIFT;
    }

    /** Set physical address pointed by the entry.
     * @param pa Physical address to set.
     * @return Previous value of physical address in the entry.
     */
    inline paddr_t SetAddress(paddr_t pa) {
        ASSERT((pa & ((1 << PAGE_SHIFT) - 1)) == 0);
        paddr_t prevPa = _ptr.entryPage->pa << PAGE_SHIFT;
        _ptr.entryPage->pa = pa >> PAGE_SHIFT;
        return prevPa;
    }

    /** Clear mapping provided by the entry. */
    inline void Clear() { *_ptr.raw = 0; }

    /** Cast entry to physical address. The same effect as from @ref GetAddress.
     * @return Physical address pointed by the entry.
     */
    inline operator paddr_t() { return GetAddress(); }

    /** Set physical address pointed by the entry. The same effect as from
     * @ref SetAddress.
     * @param pa Physical address to set.
     * @return Reference to itself.
     */
    inline LatEntry &operator=(paddr_t pa) { SetAddress(pa); return *this; }

    /** Cast entry to pointer.
     * @return Pointer to the entry in a table.
     */
    inline operator void *() { return _ptr.ptr; }

    /** Get process context identifier. Not all architectures support this
     * parameter. Zero returned on unsupported architectures. The entry must
     * beLatEntrytables root entry.
     * @return Process context identifier associated with the given root entry.
     */
    inline ProcCtxId GetProcCtxId() {
        ENSURE(_tableLvl == NUM_LAT_TABLES);
        return _ptr.cr3->pcid;
    }

    /** Set process context identifier. Not all architectures support this
     * parameter. The value is ignored on unsupported architectures. The entry
     * must beLatEntrytables root entry.
     * @return Process context identifier associated with the given root entry.
     */
    inline ProcCtxId SetProcCtxId(ProcCtxId pcid) {
        ENSURE(_tableLvl == NUM_LAT_TABLES);
        ProcCtxId oldPcid = _ptr.cr3->pcid;
        if (vmCaps.IsValid() && vmCaps.pcid) {
            _ptr.cr3->pcid = pcid;
        }
        return oldPcid;
    }

    /** Switches current address space to the specified root. Entry must be
     * new address space root entry.
     */
    inline void Activate() {
        ENSURE(_tableLvl == NUM_LAT_TABLES);
        cpu::wcr3(*_ptr.raw);
    }

private:

    /** CR3 register format. */
    struct EntryCr3 {
        union {
            /* CR4.PCIDE = 0 */
            paddr_t :3,
                    pwt:1,
                    pcd:1,
                    :7,
                    pa:40,
                    :12;
            /* CR4.PCIDE = 1 */
            paddr_t pcid:12;
        };
    };

    /**LatEntrytable entry which maps page. */
    struct EntryPage {
        paddr_t     /** Page is present. */
                    present:1,
                    /** Page is writable. */
                    write:1,
                    /** Page is accessible from user-land. */
                    user:1,
                    /** Writing to the page bypasses the CPU cache. */
                    writeThrough:1,
                    /** Caching for the page is disabled. */
                    cacheDisable:1,
                    /** Page has been accessed. Set by hardware, should be
                    * cleared by software.
                    */
                    accessed:1,
                    /** Page content has been modified. Set by hardware,
                     * should be cleared by software.
                     */
                    dirty:1,
                    /** Page memory type. */
                    pat:1,
                    /** Page is global - can be used by TLB in all address
                     * spaces.
                     */
                    global:1,
                    :3,
                    /** Page physical address. */
                    pa:40,
                    :11,
                    /** Instruction fetching from the page is disabled. */
                    executeDisable:1;
    };

    /**LatEntrytable entry which maps another table. */
    struct EntryTable {
        paddr_t     /** Page is present. */
                    present:1,
                    /** Page is writable. */
                    write:1,
                    /** Page is accessible from user-land. */
                    user:1,
                    /** Writing to the page bypasses the CPU cache. */
                    writeThrough:1,
                    /** Caching for the page is disabled. */
                    cacheDisable:1,
                    /** Page has been accessed. Set by hardware, should be
                    * cleared by software.
                    */
                    accessed:1,
                    :1,
                     /** Page Size - the entry maps the data page, bypassing
                      * all the restLatEntrytables in the hierarchy. Size of
                      * the page is equal to the size of the region
                      * controlled by the entry. Should be zero for this
                      * structure. @ref EntryPage describes the structure
                      * when it is not zero.
                      */
                    pageSize:1,
                    :4,
                    /** Page physical address. */
                    pa:40,
                    :11,
                    /** Instruction fetching from the page is disabled. */
                    executeDisable:1;
    };

    volatile union EntryPtr {
        void *ptr;
        paddr_t *raw;
        EntryCr3 *cr3;
        EntryPage *entryPage;
        EntryTable *entryTable;
    } _ptr;

    u32 _tableLvl;
};

/** Invalidate virtual address mapping. Flushes TLB entry for this address if
 * exists.
 * @param va Virtual address to invalidate.
 */
inline void
InvalidateVaddr(vaddr_t va)
{
    cpu::invlpg(va);
}

/** Initialize paging on the current CPU. */
inline void
InitPaging(bool enablePaging)
{
    if (enablePaging) {
        u64 cr0 = cpu::rcr0();
        if (!(cr0 & cpu_reg::CR0_PG)) {
            cpu::wcr0(cr0 | cpu_reg::CR0_PG);
        }
    } else {
        cpu::CpuCaps caps;

        /* Enable "execute-disable" feature if available. */
        if (caps.GetCapability(cpu::CPU_CAP_PG_NX)) {
            cpu::wrmsr(cpu_reg::MSR_IA32_EFER,
                       cpu::rdmsr(cpu_reg::MSR_IA32_EFER) | cpu_reg::IA32_EFER_NXE);
        }

        u64 features = cpu::rcr4();
        /* Enable global pages if available. */
        if (caps.GetCapability(cpu::CPU_CAP_PG_PGE)) {
            features |= cpu_reg::CR4_PGE;
        }
        /* Enable process context identification if available. */
        if (caps.GetCapability(cpu::CPU_CAP_PG_PCID)) {
            features |= cpu_reg::CR4_PCDIE;
        }
        cpu::wcr4(features);
    }
}

} /* namespace vm */

#endif /* MD_VM_H_ */
