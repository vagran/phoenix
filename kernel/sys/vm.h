/*
 * /phoenix/kernel/sys/vm.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef VM_H_
#define VM_H_

/** @file vm.h
 * Virtual memory machine independent definitions.
 */

/** Namespace with virtual memory subsystem components. */
namespace
#ifndef AUTONOMOUS_LINKING
vm
#endif /* AUTONOMOUS_LINKING */
{

/** Machine-independent flags for each PAT table entry.
 * Underlying machine-dependent implementation translates them to real
 * machine-dependent values for each table in the hierarchy. Some tables (and/or
 * some architectures) may not support some flags, so they are ignored.
 */
enum PatEntryFlags {
    /** Indicates that a page is resident. If not set the rest
     * parameters are not valid; access to this page will cause page fault or
     * TLB miss fault depending on architecture.
     */
    PAT_EF_PRESENT =        0x1,
    /** Indicates that a page is writable. If not set then the page is read-only
     * and write access will produce page fault.
     */
    PAT_EF_WRITE =          0x2,
    /** Indicates that a page is accessible by user-land. If not set then the
     * page is accessible by system only.
     */
    PAT_EF_USER =           0x4,
    /** Indicates that writing to a page is not cached and causes immediate
     * writing to physical memory.
     */
    PAT_EF_WRITE_THROUGH =  0x8,
    /** Indicates that this page is never cached. Can be useful for memory
     * mapped devices.
     */
    PAT_EF_CACHE_DISABLE =  0x10,
    /** Indicates that this is accessible for instructions fetches. */
    PAT_EF_EXECUTE =        0x20,
    /** Indicates that this page is global. It could provide a hint for
     * underlying PAT implementation to preserve cached entries for such pages
     * when switching virtual address spaces.
     */
    PAT_EF_GLOBAL =         0x40,
};

} /* namespace vm */

#include <md_vm.h>

namespace
#ifndef AUTONOMOUS_LINKING
vm
#endif /* AUTONOMOUS_LINKING */
{

enum {
    /** Memory page size in bytes. */
    PAGE_SIZE =     (1 << PAGE_SHIFT),
};

/** Class representing generic address of the VM subsystem. */
template <typename AddrType>
class Addr {
public:
    /** Construct VM address from VM address integer type. */
    inline Addr(AddrType addr = 0) { _addr.addr = addr; }
    /** Construct VM address from integer type. */
    inline Addr(int addr) { _addr.addr = addr; }
    /** Construct VM address from pointer type. */
    inline Addr(void *ptr) { _addr.ptr = ptr; }

    /** Assign address value to VM address. */
    inline Addr &operator=(AddrType addr) { _addr.addr = addr; return *this; }
    /** Assign pointer value to VM address. */
    inline Addr &operator=(void *ptr) { _addr.ptr = ptr; return *this; }

    /** Add another VM address to the current one. */
    inline Addr &operator+=(const Addr &addr) { _addr.addr += addr._addr.addr; return *this; }
    /** Add another VM address to the current one. */
    inline Addr &operator+=(const AddrType addr) { _addr.addr += addr; return *this; }

    /** Subtract another VM address from the current one. */
    inline Addr &operator-=(const Addr &addr) { _addr.addr -= addr._addr.addr; return *this; }
    /** Subtract another VM address from the current one. */
    inline Addr &operator-=(const AddrType addr) { _addr.addr -= addr; return *this; }

    /** Compare VM addresses. */
    inline bool operator==(const Addr &addr) { return _addr.addr == addr._addr.addr; }
    /** Compare VM addresses. */
    inline bool operator==(void *ptr) { return _addr.ptr == ptr; }
    /** Compare VM addresses. */
    inline bool operator==(AddrType addr) { return _addr.addr == addr; }
    /** Compare VM addresses. */
    inline bool operator==(int addr) { return _addr.addr == static_cast<AddrType>(addr); }

    /** Compare VM addresses. */
    inline bool operator!=(const Addr &addr) { return _addr.addr != addr._addr.addr; }
    /** Compare VM addresses. */
    inline bool operator!=(void *ptr) { return _addr.ptr != ptr; }
    /** Compare VM addresses. */
    inline bool operator!=(AddrType addr) { return _addr.addr != addr; }
    /** Compare VM addresses. */
    inline bool operator!=(int addr) { return _addr.addr != static_cast<AddrType>(addr); }

    /** Cast VM address value to base address type. */
    inline operator AddrType() { return _addr.addr; }
    /** Return VM address base type value. */
    inline AddrType BaseAddr() { return _addr.addr; }
    /** Cast VM address value to a generic pointer. */
    inline operator void *() { return _addr.ptr; }
    /** Cast VM address value to a pointer of specified type */
    template<typename T>
    inline operator T *() { return static_cast<T *>(_addr.ptr); }

    /** Round up VM address.
     *
     * Template parameter @a alignment specifies an alignment to apply for
     * rounding.
     * @return Reference to the same object.
     */
    inline Addr &RoundUp(AddrType alignment = PAGE_SIZE) {
        _addr.addr = ::RoundUp2(_addr.addr, alignment);
        return *this;
    }

    /** Round down VM address.
     *
     * Template parameter @a alignment specifies an alignment to apply for
     * rounding.
     * @return Reference to the same object.
     */
    inline Addr &RoundDown(AddrType alignment = PAGE_SIZE) {
        _addr.addr = ::RoundDown2(_addr.addr, alignment);
        return *this;
    }

    /** Get page index which corresponds to a given virtual address. */
    inline PageIdx GetPageIdx() { return _addr.addr >> PAGE_SHIFT; }

    /** Get offset in the page which corresponds to a given virtual address. */
    inline vaddr_t GetPageOffset() {
        VaddrDecoder dec(_addr.addr);
        return dec.GetPageOffset();
    }

protected:
    union {
        AddrType addr;
        void *ptr;
    } _addr;
};

/** Class representing virtual address type. */
class Vaddr : public Addr<vaddr_t> {
public:
    /** Construct virtual address from virtual address integer type. */
    inline Vaddr(vaddr_t addr = 0) : Addr(addr) { }
    /** Construct virtual address from integer type. */
    inline Vaddr(int addr) : Addr(addr) { }
    /** Construct virtual address from pointer type. */
    inline Vaddr(void *ptr) : Addr(ptr) { }
    /** Construct virtual address from the base class. */
    inline Vaddr(Addr<vaddr_t> addr) : Addr(addr) { }

    /** Return physical address for identity mapping. */
    inline paddr_t IdentityPaddr() { return static_cast<paddr_t>(_addr.addr); }
};

/** Class representing physical address type. */
class Paddr : public Addr<paddr_t> {
public:
    /** Construct physical address from physical address integer type. */
    inline Paddr(paddr_t addr = 0) : Addr(addr) { }
    /** Construct physical address from integer type. */
    inline Paddr(int addr) : Addr(addr) { }
    /** Construct physical address from pointer type. */
    inline Paddr(void *ptr) : Addr(ptr) { }
    /** Construct physical address from the base class. */
    inline Paddr(Addr<paddr_t> addr) : Addr(addr) { }

    /** Return virtual address for identity mapping. */
    inline vaddr_t IdentityVaddr() { return static_cast<vaddr_t>(_addr.addr); }
};

} /* namespace vm */

#endif /* VM_H_ */
