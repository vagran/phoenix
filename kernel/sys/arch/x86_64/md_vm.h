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

namespace
#ifndef AUTONOMOUS_LINKING
vm
#endif /* AUTONOMOUS_LINKING */
{

enum {
    /** Number of bits to shift to get a memory page frame. */
    PAGE_SHIFT =            12,

    /** Number of physical address translation tables in the hierarchy. */
    NUM_PAT_TABLES =        4,
};

/** Memory page index. */
typedef u64 PageIdx;

/** Index of an entry in a physical address translation table. */
typedef u32 PatEntryIdx;

/** Helper class to extract components (PAT tables entries indices and offset
 * in a page) from a virtual address. Defines the layout of PAT tables
 * hierarchy and virtual address fields layout.
 */
class VaddrDecoder {
public:
    /** Construct decoder object from virtual address. */
    inline VaddrDecoder(vaddr_t va = 0) { _va.va = va; }

    /** Get size of specified PAT table.
     *
     * @param tableLvl Null-based table level starting from least significant
     *      (e.g. page table, page directory, ...).
     * @return Number of entries in a specified table.
     */
    inline u32 GetTableSize(u32 tableLvl) {
        ASSERT(tableLvl < NUM_PAT_TABLES);
        /* 512 entries in each PAT table. */
        return 512;
    }

    /** Get entry index in a specified PAT table of the given virtual address.
     *
     * @param tableLvl PAT table level.
     * @return Index of the entry in the specified PAT table corresponding to
     *      the given virtual address.
     */
    inline PatEntryIdx GetEntryIndex(u32 tableLvl) {
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

    union {
        vaddr_t va;
        VaFields fields;
    } _va;
};

/** Class representing PAT table entry. */
class PatEntry {
public:
    /** Create entry by a table and virtual address. The entry in the table
     * selected accordingly to virtual address provided.
     * @param va Virtual address.
     * @param table Pointer to the table.
     * @param tableLvl Null-based table level starting from least significant
     *      (e.g. page table, page directory, ...).
     */
    inline PatEntry(vaddr_t va, void *table, u32 tableLvl = 0) {
        _tableLvl = tableLvl;
        _ptr.ptr = table;
        VaddrDecoder dec(va);
        _ptr.raw += dec.GetEntryIndex(tableLvl);
    }

    /** Create entry by direct pointer.
     *
     * @param entry Pointer to the entry.
     * @param tableLvl Level of the table which contains the entry.
     */
    inline PatEntry(void *entry, u32 tableLvl = 0) {
        ASSERT(tableLvl <= NUM_PAT_TABLES);
        _tableLvl = tableLvl;
        _ptr.ptr = entry;
    }

    /** Check if the page mapped by the entry was accessed since last flag
     * reset.
     * @return @a true if the page was accessed.
     */
    inline bool IsAccessed() {
        if (_tableLvl == NUM_PAT_TABLES) {
            /* CR3 does not have the flag. */
            return false;
        }
        return _ptr.entryPage->accessed;
    }

    /** Set or clear @a accessed flag for an entry.
     * @param flag Value to set in the flag.
     *
     * @return previous value.
     */
    inline bool SetAccessed(bool flag = true) {
        if (_tableLvl == NUM_PAT_TABLES) {
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
        if (_tableLvl == NUM_PAT_TABLES) {
            /* CR3 does not have the flag. */
            return false;
        }
        return _ptr.entryPage->dirty;
    }

    /** Set or clear @a dirty flag for an entry.
     * @param flag Value to set in the flag.
     *
     * @return previous value.
     */
    inline bool SetDirty(bool flag = true) {
        if (_tableLvl == NUM_PAT_TABLES) {
            /* CR3 does not have the flag. */
            return false;
        }
        bool curFlag = _ptr.entryPage->dirty;
        _ptr.entryPage->dirty = flag;
        return curFlag;
    }

    /** Set or clear specified flag in PAT entry.
     *
     * @param flag Flag to set or clear. Corresponding machine-dependent flag
     *      will be affected.
     * @param setIt Set the flag if @a true, clear otherwise.
     * @return Previous flag state.
     */
    bool SetFlag(PatEntryFlags flag, bool setIt = true) {
        bool prev;
        ASSERT(_tableLvl <= NUM_PAT_TABLES);
        if (_tableLvl == NUM_PAT_TABLES) {
            switch (flag) {
            case PAT_EF_WRITE_THROUGH:
                prev = _ptr.cr3->pwt;
                _ptr.cr3->pwt = setIt ? 1 : 0;
                break;
            case PAT_EF_CACHE_DISABLE:
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
        case PAT_EF_PRESENT:
            prev = _ptr.entryPage->present;
            _ptr.entryPage->present = setIt ? 1 : 0;
            break;
        case PAT_EF_WRITE:
            prev = _ptr.entryPage->write;
            _ptr.entryPage->write = setIt ? 1 : 0;
            break;
        case PAT_EF_USER:
            prev = _ptr.entryPage->user;
            _ptr.entryPage->user = setIt ? 1 : 0;
            break;
        case PAT_EF_WRITE_THROUGH:
            prev = _ptr.entryPage->writeThrough;
            _ptr.entryPage->writeThrough = setIt ? 1 : 0;
            break;
        case PAT_EF_CACHE_DISABLE:
            prev = _ptr.entryPage->cacheDisable;
            _ptr.entryPage->cacheDisable = setIt ? 1 : 0;
            break;
        case PAT_EF_EXECUTE:
            prev = !_ptr.entryPage->executeDisable;
            _ptr.entryPage->executeDisable = setIt ? 0 : 1;
            break;
        case PAT_EF_GLOBAL:
            prev = _ptr.entryPage->global;
            _ptr.entryPage->global = setIt ? 1 : 0;
            break;
        default:
            FAULT("Invalid flag specified: %d", flag);
            break;
        }
        return prev;
    }

    /** Set PAT entry flags.
     *
     * @param flags Flags which must be set. Any number of flags defined by
     *      @ref PatEntryFlags can be specified. Corresponding machine-dependent
     *      flags will be set, flags which were not specified will be cleared.
     * @return Previous combination of the flags.
     */
    long SetFlags(long flags) {
        static const PatEntryFlags allFlags[] = {
            PAT_EF_PRESENT,
            PAT_EF_WRITE,
            PAT_EF_USER,
            PAT_EF_WRITE_THROUGH,
            PAT_EF_CACHE_DISABLE,
            PAT_EF_EXECUTE,
            PAT_EF_GLOBAL
        };
        long prev;

        for (auto flag : allFlags) {
            if (SetFlag(flag, flags & flag)) {
                prev |= flag;
            }
        }
        return prev;
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

    /** PAT table entry which maps page. */
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

    /** PAT table entry which maps another table. */
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
                      * all the rest PAT tables in the hierarchy. Size of
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

    union EntryPtr {
        void *ptr;
        paddr_t *raw;
        EntryCr3 *cr3;
        EntryPage *entryPage;
        EntryTable *entryTable;
    } _ptr;

    u32 _tableLvl;
};

} /* namespace vm */

#endif /* MD_VM_H_ */
