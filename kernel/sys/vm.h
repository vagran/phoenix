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

#ifdef AUTONOMOUS_LINKING
namespace {
#endif /* AUTONOMOUS_LINKING */

/** Namespace with virtual memory subsystem components. */
namespace vm {

/** Machine-independent flags for each LatEntrytable entry.
 * Underlying machine-dependent implementation translates them to real
 * machine-dependent values for each table in the hierarchy. Some tables (and/or
 * some architectures) may not support some flags, so they are ignored.
 */
enum LatEntryFlags {
    /** Indicates that a page is resident. If not set the rest
     * parameters are not valid; access to this page will cause page fault or
     * TLB miss fault depending on architecture.
     */
    LAT_EF_PRESENT =        0x1,
    /** Indicates that a page is writable. If not set then the page is read-only
     * and write access will produce page fault.
     */
    LAT_EF_WRITE =          0x2,
    /** Indicates that a page is accessible by user-land. If not set then the
     * page is accessible by system only.
     */
    LAT_EF_USER =           0x4,
    /** Indicates that writing to a page is not cached and causes immediate
     * writing to physical memory.
     */
    LAT_EF_WRITE_THROUGH =  0x8,
    /** Indicates that this page is never cached. Can be useful for memory
     * mapped devices.
     */
    LAT_EF_CACHE_DISABLE =  0x10,
    /** Indicates that this is accessible for instructions fetches. */
    LAT_EF_EXECUTE =        0x20,
    /** Indicates that this page is global. It could provide a hint for
     * underlying LAT entry (see @ref LatEntry) implementation to preserve
     * cached entries for such pages when switching virtual address spaces.
     */
    LAT_EF_GLOBAL =         0x40,
};

/** Virtual memory subsystem capabilities. @ref IsValid method should be called
 * before accessing the structure data member.
 */
#ifndef AUTONOMOUS_LINKING
extern
#endif /* AUTONOMOUS_LINKING */
struct VmCaps {
    bool    valid:1,    /**< The structure is initialized if true. */
            pge:1,      /**< Global-page support. */
            pat:1,      /**< Page-attribute table. */
            pcid:1,     /**< Process-context identifiers. */
            smep:1,     /**< Supervisor-mode execution prevention. */
            nx:1,       /**< Execute disable. */
            oneGb:1;    /**< 1-GByte pages. */
    short   width_phys; /**< Physical address width. */
    short   width_lin;  /**< Linear address width. */

    /** Check if data members are valid.
     *
     * @return @a true if data members are valid and can be accessed.
     */
    inline bool IsValid() {
        if (!valid) {
            Initialize();
        }
        return true;
    }

    void Initialize() {
        cpu::CpuCaps caps;

        pge = caps.GetCapability(cpu::CPU_CAP_PG_PGE) ? true : false;
        pat = caps.GetCapability(cpu::CPU_CAP_PG_PAT) ? true : false;
        pcid = caps.GetCapability(cpu::CPU_CAP_PG_PCID) ? true : false;
        smep = caps.GetCapability(cpu::CPU_CAP_PG_SMEP) ? true : false;
        nx = caps.GetCapability(cpu::CPU_CAP_PG_NX) ? true : false;
        oneGb = caps.GetCapability(cpu::CPU_CAP_PG_1GB) ? true : false;
        width_phys = caps.GetCapability(cpu::CPU_CAP_PG_WIDTH_PHYS);
        width_lin = caps.GetCapability(cpu::CPU_CAP_PG_WIDTH_LIN);

        valid = true;
    }

} vmCaps;

} /* namespace vm */

#include <md_vm.h>

namespace vm {

enum {
    /** Memory page size in bytes. */
    PAGE_SIZE =             (1 << PAGE_SHIFT),
    /** Number of quick map entries. */
    NUM_QUICK_MAP =         4,

    /** System data space region size. */
    SYS_DATA_SIZE =         4ul * 1024ul * 1024ul * 1024ul,
    /** Size of of gate area region. Code for the kernel mode entry points is
     * placed in this area.
     */
    GATE_AREA_SIZE =        64 * 1024,

    /** Container code first address. */
    VMA_CNTR_TEXT =         PAGE_SIZE,
    /** Kernel code first address. */
    VMA_KERNEL_TEXT =       KERNEL_ADDRESS,
    /** System data space start address. */
    VMA_SYS_DATA =          2ul * 1024ul * 1024ul * 1024ul,
    /** Kernel gate area start address. */
    VMA_KERNEL_PUBLIC =     VMA_SYS_DATA - GATE_AREA_SIZE,
    /** Start address of global data space. */
    VMA_GLOBAL_DATA =       VMA_SYS_DATA + SYS_DATA_SIZE
};

/** Class representing generic address of the VM subsystem. */
template <typename AddrType>
class Addr {
public:
    /** Construct VM address from VM address integer type. */
    inline Addr(AddrType addr = 0) { _addr.addr = addr; }
    /** Construct VM address from any integer type. */
    template <typename T>
    inline Addr(T addr) { _addr.addr = addr; }
    /** Construct VM address from pointer type. */
    template <typename T>
    inline Addr(T *ptr) { _addr.ptr = ptr; }

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

    /** Compare VM addresses. */
    inline bool operator <(Addr &addr) { return _addr.addr < addr._addr.addr; }
    /** Compare VM addresses. */
    template <typename T>
    inline bool operator <(T addr) { return _addr.addr < addr; }
    /** Compare VM addresses. */
    inline bool operator <=(Addr &addr) { return _addr.addr <= addr._addr.addr; }
    /** Compare VM addresses. */
    template <typename T>
    inline bool operator <=(T addr) { return _addr.addr <= addr; }
    /** Compare VM addresses. */
    inline bool operator >(Addr &addr) { return _addr.addr > addr._addr.addr; }
    /** Compare VM addresses. */
    template <typename T>
    inline bool operator >(T addr) { return _addr.addr > addr; }
    /** Compare VM addresses. */
    inline bool operator >=(Addr &addr) { return _addr.addr >= addr._addr.addr; }
    /** Compare VM addresses. */
    template <typename T>
    inline bool operator >=(T addr) { return _addr.addr >= addr; }

    /** Sum VM addresses. */
    inline Addr operator +(Addr &addr) {
        return Addr(_addr.addr + addr._addr.addr);
    }
    /** Sum VM addresses. */
    template <typename T>
    inline Addr operator +(T addr) {
        return Addr(_addr.addr + addr);
    }
    /** Subtract VM addresses. */
    inline Addr operator -(Addr &addr) {
        return Addr(_addr.addr - addr._addr.addr);
    }
    /** Subtract VM addresses. */
    template <typename T>
    inline Addr operator -(T addr) {
        return Addr(_addr.addr - addr);
    }

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

    /** Check if the address aligned to specified alignment value.
     *
     * @param alignment Alignment value, must be power of two.
     * @return @a true if address  if aligned, @a false otherwise.
     */
    inline bool IsAligned(AddrType alignment = PAGE_SIZE) {
        ASSERT(IsPowerOf2(alignment));
        return !(_addr.addr & (alignment - 1));
    }

    /** Get page index which corresponds to a given virtual address. */
    inline PageIdx GetPageIdx() { return _addr.addr >> PAGE_SHIFT; }

    /** Get offset in the page which corresponds to a given virtual address. */
    inline vaddr_t GetPageOffset() {
        VaddrDecoder dec(_addr.addr);
        return dec.GetPageOffset();
    }

    /** Support for converting this class to string via @ref text_stream::OTextStream. */
    inline bool CheckFmtChar(char fmtChar) {
        return !fmtChar || fmtChar == 'd' ||  fmtChar == 'u' || fmtChar == 'p' ||
               fmtChar == 'x' || fmtChar == 'X' || fmtChar == 'o';
    }

    /** Support for converting this class to string via @ref text_stream::OTextStream. */
    inline bool ToString(text_stream::OTextStreamBase &stream,
                         text_stream::OTextStreamBase::Context &ctx,
                         char fmtChar = 0) {

        if (fmtChar == 'p') {
            return stream.FormatValue(ctx,  _addr.ptr, fmtChar);
        } else if (!fmtChar) {
            fmtChar = 'x';
        }
        return stream.FormatValue(ctx, _addr.addr, fmtChar);
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
    /** Construct virtual address from any integer type. */
    template <typename T>
    inline Vaddr(T addr) : Addr(addr) { }
    /** Construct virtual address from pointer type. */
    template <typename T>
    inline Vaddr(T *ptr) : Addr(ptr) { }
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
    /** Construct physical address from any integer type. */
    template <typename T>
    inline Paddr(T addr) : Addr(addr) { }
    /** Construct physical address from pointer type. */
    template <typename T>
    inline Paddr(T *ptr) : Addr(ptr) { }
    /** Construct physical address from the base class. */
    inline Paddr(Addr<paddr_t> addr) : Addr(addr) { }

    /** Return virtual address for identity mapping. */
    inline vaddr_t IdentityVaddr() { return static_cast<vaddr_t>(_addr.addr); }
};

} /* namespace vm */

#include <vm_mm.h>

#ifdef AUTONOMOUS_LINKING
}
#endif /* AUTONOMOUS_LINKING */

#endif /* VM_H_ */
