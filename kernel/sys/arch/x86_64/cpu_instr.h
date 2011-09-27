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
    u8 _v;

    ASM ("inb %w1, %0" : "=a"(_v) : "Nd"(port));
    return _v;
}

__inline u16
inw(u16 port)
{
    u16 _v;

    ASM ("inw %w1, %0" : "=a"(_v) : "Nd"(port));
    return _v;
}

__inline u32
inl(u16 port)
{
    u32 _v;

    ASM ("inl %w1, %0" : "=a"(_v) : "Nd"(port));
    return _v;
}

__inline void
outb(u16 port, u8 value)
{
    ASM ("outb %b0, %w1" : : "a"(value), "Nd"(port));
}

__inline void
outw(u16 port, u16 value)
{
    ASM ("outw %w0, %w1": :"a"(value), "Nd"(port));

}

__inline void
outl(u16 port, u32 value)
{
    ASM ("outl %0, %w1" : : "a"(value), "Nd" (port));
}

__inline void
invlpg(vaddr_t va)
{
    ASM ("invlpg %0" : : "m"(*(u8 *)va));
}

__inline u32
rcr0()
{
    register u64 r;

    ASM ("mov %%cr0, %0" : "=r"(r));
    return r;
}

__inline void
wcr0(u64 x)
{
    ASM ("mov %0, %%cr0" : : "r"(x));
}

__inline u32
rcr2()
{
    register u64 r;

    ASM ("mov %%cr2, %0" : "=r"(r));
    return r;
}

__inline void
wcr2(u32 x)
{
    ASM ("mov %0, %%cr2" : : "r"(x));
}

__inline u64
rcr3()
{
    register u64 r;

    ASM ("mov %%cr3, %0" : "=r"(r));
    return r;
}

__inline void
wcr3(u64 x)
{
    ASM ("mov %0, %%cr3" : : "r"(x));
}

__inline u32
rcr4()
{
    register u64 r;

    ASM ("mov %%cr4, %0" : "=r"(r));
    return r;
}

__inline void
wcr4(u64 x)
{
    ASM ("mov %0, %%cr4" : : "r"(x));
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
        "lgdt   %0"
        :
        : "m"(*(u8 *)p)
        );
}

__inline void
lidt(void *p)
{
    ASM (
        "lidt   %0"
        :
        : "m"(*(u8 *)p)
        );
}

__inline void
lldt(u16 sel)
{
    ASM (
        "lldt   %0"
        :
        : "r"(sel)
        );
}

__inline void
sgdt(void *p)
{
    ASM (
        "sgdt   %0"
        : "=m"(*(u8 *)p)
    );
}

__inline void
sidt(void *p)
{
    ASM (
        "sidt   %0"
        : "=m"(*(u8 *)p)
    );
}

__inline u16
sldt()
{
    u16 sel;
    ASM (
        "sldt   %0"
        : "=r"(sel)
    );
    return sel;
}

__inline void
ltr(u16 sel)
{
    ASM (
        "ltr    %0"
        :
        : "r"(sel)
    );
}

__inline u16
str()
{
    u16 sel;
    ASM (
        "str    %0"
        : "=&r"(sel)
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
        "popq %0\n"
        : "=r"(rc)
    );
    return rc;
}

__inline void
SetFlags(u64 value)
{
    ASM (
        "pushq %0\n"
        "popfq\n"
        :
        : "r"(value)
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
