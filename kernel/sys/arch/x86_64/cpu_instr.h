/*
 * /phoenix/kernel/sys/arch/x86_64/cpu_instr.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef CPU_INSTR_H_
#define CPU_INSTR_H_

/** @file cpu_instr.h
 * Machine-dependent definitions of CPU instructions.
 *
 * This file contains C wrappers around separate CPU instructions.
 */

#ifdef AUTONOMOUS_LINKING
namespace {
#endif /* AUTONOMOUS_LINKING */

namespace cpu {

inline u64
bsf(u64 string)
{
    u64 rc;

    ASM ("bsfq %[string], %[rc]" : [rc]"=r"(rc) : [string]"r"(string));
    return rc;
}

inline u8
inb(u16 port)
{
    u8 value;

    ASM ("inb %w[port], %[value]" : [value]"=a"(value) : [port]"Nd"(port));
    return value;
}

inline u16
inw(u16 port)
{
    u16 value;

    ASM ("inw %w[port], %[value]" : [value]"=a"(value) : [port]"Nd"(port));
    return value;
}

inline u32
inl(u16 port)
{
    u32 value;

    ASM ("inl %w[port], %[value]" : [value]"=a"(value) : [port]"Nd"(port));
    return value;
}

inline void
outb(u16 port, u8 value)
{
    ASM ("outb %b[value], %w[port]" : : [value]"a"(value), [port]"Nd"(port));
}

inline void
outw(u16 port, u16 value)
{
    ASM ("outw %w[value], %w[port]": : [value]"a"(value), [port]"Nd"(port));

}

inline void
outl(u16 port, u32 value)
{
    ASM ("outl %[value], %w[port]" : : [value]"a"(value), [port]"Nd"(port));
}

inline void
invlpg(vaddr_t va)
{
    ASM ("invlpg %[va]" : : [va]"m"(*static_cast<u8 *>(reinterpret_cast<void *>(va))));
}

inline u64
rcr0()
{
    register u64 r;

    ASM ("mov %%cr0, %[r]" : [r]"=r"(r));
    return r;
}

inline void
wcr0(u64 x)
{
    ASM ("mov %[x], %%cr0" : : [x]"r"(x));
}

inline u64
rcr2()
{
    register u64 r;

    ASM ("mov %%cr2, %[r]" : [r]"=r"(r));
    return r;
}

inline void
wcr2(u64 x)
{
    ASM ("mov %[x], %%cr2" : : [x]"r"(x));
}

inline u64
rcr3()
{
    register u64 r;

    ASM ("mov %%cr3, %[r]" : [r]"=r"(r));
    return r;
}

inline void
wcr3(u64 x)
{
    ASM ("mov %[x], %%cr3" : : [x]"r"(x));
}

inline u64
rcr4()
{
    register u64 r;

    ASM ("mov %%cr4, %[r]" : [r]"=r"(r));
    return r;
}

inline void
wcr4(u64 x)
{
    ASM ("mov %[x], %%cr4" : : [x]"r"(x));
}

inline void
cpuid(u32 op, u32 subop, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    u32 _eax, _ebx, _ecx, _edx;
    ASM (
        "cpuid"
        : "=a"(_eax), "=b"(_ebx), "=c"(_ecx), "=d"(_edx)
        : "a"(op), "c"(subop)
    );
    if (eax) {
        *eax = _eax;
    }
    if (ebx) {
        *ebx = _ebx;
    }
    if (ecx) {
        *ecx = _ecx;
    }
    if (edx) {
        *edx = _edx;
    }
}

inline u64
rdtsc()
{
    u64 rcL, rcH;

    ASM ("rdtsc" : "=a"(rcL), "=d"(rcH));
    return rcL | (rcH << 32);
}

inline void
hlt()
{
    ASM("hlt");
}

inline void
cli()
{
    ASM("cli");
}

/* Pseudo instructions */
inline u64
GetFlags()
{
    u64 rc;
    ASM (
        "pushfq\n"
        "popq %[flags]\n"
        : [flags]"=r"(rc)
    );
    return rc;
}

inline void
SetFlags(u64 value)
{
    ASM (
        "pushq %[flags]\n"
        "popfq\n"
        :
        : [flags]"r"(value)
    );
}

inline void
sti()
{
    ASM("sti");
}


inline void
lgdt(void *p)
{
    ASM (
        "lgdt   %[gdt]"
        :
        : [gdt]"m"(*static_cast<u8 *>(p))
        );
}

inline void
lidt(void *p)
{
    ASM (
        "lidt   %[idt]"
        :
        : [idt]"m"(*static_cast<u8 *>(p))
        );
}

inline void
lldt(u16 sel)
{
    ASM (
        "lldt   %[ldt]"
        :
        : [ldt]"r"(sel)
        );
}

inline void
sgdt(void *p)
{
    ASM (
        "sgdt   %[gdt]"
        : [gdt]"=m"(*static_cast<u8 *>(p))
    );
}

inline void
sidt(void *p)
{
    ASM (
        "sidt   %[idt]"
        : [idt]"=m"(*static_cast<u8 *>(p))
    );
}

inline u16
sldt()
{
    u16 sel;
    ASM (
        "sldt   %[ldt]"
        : [ldt]"=r"(sel)
    );
    return sel;
}

inline void
ltr(u16 sel)
{
    ASM (
        "ltr    %[sel]"
        :
        : [sel]"r"(sel)
    );
}

inline u16
str()
{
    u16 sel;
    ASM (
        "str    %[sel]"
        : [sel]"=&r"(sel)
    );
    return sel;
}

inline u64
rdmsr(u32 msr)
{
    u64 rcL, rcH;
    ASM (
        "rdmsr"
        : "=a"(rcL), "=d"(rcH)
        : "c"(msr)
    );
    return rcL | (rcH << 32);
}

inline void
wrmsr(u32 msr, u64 value)
{
    ASM (
        "wrmsr"
        :
        : "c"(msr), "a"(value & 0xffffffff), "d"(value >> 32)
    );
}

inline void
sysenter()
{
    ASM ("sysenter");
}

inline void
sysexit(u32 eip, u32 esp)
{
    ASM (
        "sysexit"
        :
        : "d"(eip), "c"(esp)
    );
}

inline void
pause()
{
    ASM ("pause");
}

inline void
lfence()
{
    ASM ("lfence");
}

inline void
sfence()
{
    ASM ("sfence");
}

inline void
mfence()
{
    ASM ("mfence");
}

/** Pseudo-instruction - pause CPU for short time to use in spin loops. Should
 * be defined in all architectures.
 */
inline void
Pause()
{
    pause();
}

/** Pseudo-instruction - stop CPU completely. Should be defined in all
 * architectures.
 */
inline void Halt() __NORETURN;
inline void
Halt()
{
    cli();
    while (true) {
        hlt();
    }
}

/** Pseudo-instruction - disable CPU interrupts. Should be defined in all
 * architectures.
 *
 * @return Previous status of interrupts - @a true if the interrupts were
 *      enabled, @a false otherwise.
 */
inline bool
DisableInterrupts()
{
    bool ret = GetFlags() & cpu_reg::EFLAGS_IF;
    cli();
    return ret;
}

/** Pseudo-instruction - enable CPU interrupts. Should be defined in all
 * architectures.
 */
inline void
EnableInterrupts()
{
    sti();
}

} /* namespace cpu */

#ifdef AUTONOMOUS_LINKING
}
#endif /* AUTONOMOUS_LINKING */

#endif /* CPU_INSTR_H_ */
