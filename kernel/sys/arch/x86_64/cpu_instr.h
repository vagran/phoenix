/*
 * /phoenix/kernel/sys/arch/x86_64/cpu_instr.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
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

namespace {

__inline u64
bsf(u64 string)
{
    u64 rc;

    ASM ("bsfq %[rc], %[string]" : [rc]"=r"(rc) : [string]"r"(string));
    return rc;
}

__inline u8
inb(u16 port)
{
    u8 value;

    ASM ("inb %w[port], %[value]" : [value]"=a"(value) : [port]"Nd"(port));
    return value;
}

__inline u16
inw(u16 port)
{
    u16 value;

    ASM ("inw %w[port], %[value]" : [value]"=a"(value) : [port]"Nd"(port));
    return value;
}

__inline u32
inl(u16 port)
{
    u32 value;

    ASM ("inl %w[port], %[value]" : [value]"=a"(value) : [port]"Nd"(port));
    return value;
}

__inline void
outb(u16 port, u8 value)
{
    ASM ("outb %b[value], %w[port]" : : [value]"a"(value), [port]"Nd"(port));
}

__inline void
outw(u16 port, u16 value)
{
    ASM ("outw %w[value], %w[port]": : [value]"a"(value), [port]"Nd"(port));

}

__inline void
outl(u16 port, u32 value)
{
    ASM ("outl %[value], %w[port]" : : [value]"a"(value), [port]"Nd"(port));
}

__inline void
invlpg(Vaddr va)
{
    ASM ("invlpg %[va]" : : [va]"m"(*static_cast<u8 *>(static_cast<void *>(va))));
}

__inline u64
rcr0()
{
    register u64 r;

    ASM ("mov %%cr0, %[r]" : [r]"=r"(r));
    return r;
}

__inline void
wcr0(u64 x)
{
    ASM ("mov %[x], %%cr0" : : [x]"r"(x));
}

__inline u64
rcr2()
{
    register u64 r;

    ASM ("mov %%cr2, %[r]" : [r]"=r"(r));
    return r;
}

__inline void
wcr2(u64 x)
{
    ASM ("mov %[x], %%cr2" : : [x]"r"(x));
}

__inline u64
rcr3()
{
    register u64 r;

    ASM ("mov %%cr3, %[r]" : [r]"=r"(r));
    return r;
}

__inline void
wcr3(u64 x)
{
    ASM ("mov %[x], %%cr3" : : [x]"r"(x));
}

__inline u64
rcr4()
{
    register u64 r;

    ASM ("mov %%cr4, %[r]" : [r]"=r"(r));
    return r;
}

__inline void
wcr4(u64 x)
{
    ASM ("mov %[x], %%cr4" : : [x]"r"(x));
}

__inline void
cpuid(u32 op, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    u32 _eax, _ebx, _ecx, _edx;
    ASM (
        "cpuid"
        : "=a"(_eax), "=b"(_ebx), "=c"(_ecx), "=d"(_edx)
        : "a"(op)
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

__inline u64
rdtsc()
{
    u64 x;

    ASM ("rdtsc" : "=r"(x));
    return x;
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

inline void
sti()
{
    ASM("sti");
}

__inline void
lgdt(void *p)
{
    ASM (
        "lgdt   %[gdt]"
        :
        : [gdt]"m"(*static_cast<u8 *>(p))
        );
}

__inline void
lidt(void *p)
{
    ASM (
        "lidt   %[idt]"
        :
        : [idt]"m"(*static_cast<u8 *>(p))
        );
}

__inline void
lldt(u16 sel)
{
    ASM (
        "lldt   %[ldt]"
        :
        : [ldt]"r"(sel)
        );
}

__inline void
sgdt(void *p)
{
    ASM (
        "sgdt   %[gdt]"
        : [gdt]"=m"(*static_cast<u8 *>(p))
    );
}

__inline void
sidt(void *p)
{
    ASM (
        "sidt   %[idt]"
        : [idt]"=m"(*static_cast<u8 *>(p))
    );
}

__inline u16
sldt()
{
    u16 sel;
    ASM (
        "sldt   %[ldt]"
        : [ldt]"=r"(sel)
    );
    return sel;
}

__inline void
ltr(u16 sel)
{
    ASM (
        "ltr    %[sel]"
        :
        : [sel]"r"(sel)
    );
}

__inline u16
str()
{
    u16 sel;
    ASM (
        "str    %[sel]"
        : [sel]"=&r"(sel)
    );
    return sel;
}

__inline u64
rdmsr(u32 msr)
{
    u64 rc;
    ASM (
        "rdmsr"
        : "=A"(rc)
        : "c"(msr)
    );
    return rc;
}

__inline void
wrmsr(u32 msr, u64 value)
{
    ASM (
        "wrmsr"
        :
        : "c"(msr), "A"(value)
    );
}

__inline void
sysenter()
{
    ASM ("sysenter");
}

__inline void
sysexit(u32 eip, u32 esp)
{
    ASM (
        "sysexit"
        :
        : "d"(eip), "c"(esp)
    );
}

/* Pseudo instructions */
__inline u64
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

__inline void
SetFlags(u64 value)
{
    ASM (
        "pushq %[flags]\n"
        "popfq\n"
        :
        : [flags]"r"(value)
    );
}

__inline void
pause()
{
    ASM ("pause");
}

__inline void
lfence()
{
    ASM ("lfence");
}

__inline void
sfence()
{
    ASM ("sfence");
}

__inline void
mfence()
{
    ASM ("mfence");
}

} /* Anonymous namespace */

#endif /* CPU_INSTR_H_ */
