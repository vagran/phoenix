/*
 * /phoenix/kernel/sys/arch/x86_64/md_efi.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file md_efi.h
 * Machine-dependent EFI definitions.
 */

#ifndef MD_EFI_H_
#define MD_EFI_H_

namespace efi {

/** Each argument for EFI services calls is represented by this class object. */
class EfiArg {
public:
    /** Argument value. For x86_64 architecture all arguments always are passed
     * as 64-bits values.
     */
    union {
        u64 raw;
        void *ptr;
    } value;

    /** Constructor for integer types. */
    template <typename T>
    inline EfiArg(T intValue) {
        value.raw = intValue;
    }

    /** Constructor for pointer types. */
    template <typename T>
    inline EfiArg(T *ptrValue) {
        value.ptr = ptrValue;
    }
};

/** Physical pointers to EFI service calls are wrapped by this class. */
class EfiCall {
public:
    /** Call without arguments. */
    inline Uintn operator()() {
        Uintn rc;
        ASM ("subq $40, %%rsp\n"
             "call *%[func]\n"
             "addq $40, %%rsp\n"
             : "=a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr))
             : "rcx", "rdx", "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with one argument. */
    inline Uintn operator()(EfiArg arg1) {
        Uintn rc;
        ASM ("subq $40, %%rsp\n"
             "call *%[func]\n"
             "addq $40, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1)
             : "rdx", "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with two arguments. */
    inline Uintn operator()(EfiArg arg1, EfiArg arg2) {
        Uintn rc;
        ASM ("subq $40, %%rsp\n"
             "call *%[func]\n"
             "addq $40, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1),
               "d"(arg2)
             : "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with three arguments. */
    inline Uintn operator()(EfiArg arg1, EfiArg arg2, EfiArg arg3) {
        Uintn rc;
        ASM ("subq $40, %%rsp\n"
             "mov %[arg3], %%r8\n"
             "call *%[func]\n"
             "addq $40, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1),
               "d"(arg2),
               [arg3]"m"(arg3)
             : "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with four arguments. */
    inline Uintn operator()(EfiArg arg1, EfiArg arg2, EfiArg arg3, EfiArg arg4) {
        Uintn rc;
        ASM ("subq $40, %%rsp\n"
             "mov %[arg3], %%r8\n"
             "mov %[arg4], %%r9\n"
             "call *%[func]\n"
             "addq $40, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1),
               "d"(arg2),
               [arg3]"m"(arg3),
               [arg4]"m"(arg4)
             : "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with five arguments. */
    inline Uintn operator()(EfiArg arg1, EfiArg arg2, EfiArg arg3, EfiArg arg4,
                            EfiArg arg5) {
        Uintn rc;
        ASM ("subq $40, %%rsp\n"
             "mov %[arg3], %%r8\n"
             "mov %[arg4], %%r9\n"
             "mov %[arg5], %%rax\n"
             "mov %%rax, 32(%%rsp)\n"
             "call *%[func]\n"
             "addq $40, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1),
               "d"(arg2),
               [arg3]"m"(arg3),
               [arg4]"m"(arg4),
               [arg5]"m"(arg5)
             : "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with six arguments. */
    inline Uintn operator()(EfiArg arg1, EfiArg arg2, EfiArg arg3, EfiArg arg4,
                            EfiArg arg5, EfiArg arg6) {
        Uintn rc;
        ASM ("subq $56, %%rsp\n"
             "mov %[arg3], %%r8\n"
             "mov %[arg4], %%r9\n"
             "mov %[arg5], %%rax\n"
             "mov %%rax, 32(%%rsp)\n"
             "mov %[arg6], %%rax\n"
             "mov %%rax, 40(%%rsp)\n"
             "call *%[func]\n"
             "addq $56, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1),
               "d"(arg2),
               [arg3]"m"(arg3),
               [arg4]"m"(arg4),
               [arg5]"m"(arg5),
               [arg6]"m"(arg6)
             : "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with seven arguments. */
    inline Uintn operator()(EfiArg arg1, EfiArg arg2, EfiArg arg3, EfiArg arg4,
                            EfiArg arg5, EfiArg arg6, EfiArg arg7) {
        Uintn rc;
        ASM ("subq $56, %%rsp\n"
             "mov %[arg3], %%r8\n"
             "mov %[arg4], %%r9\n"
             "mov %[arg5], %%rax\n"
             "mov %%rax, 32(%%rsp)\n"
             "mov %[arg6], %%rax\n"
             "mov %%rax, 40(%%rsp)\n"
             "mov %[arg7], %%rax\n"
             "mov %%rax, 48(%%rsp)\n"
             "call *%[func]\n"
             "addq $56, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1),
               "d"(arg2),
               [arg3]"m"(arg3),
               [arg4]"m"(arg4),
               [arg5]"m"(arg5),
               [arg6]"m"(arg6),
               [arg7]"m"(arg7)
             : "r8", "r9", "r10", "r11"
             );
        return rc;
    };

    /** Call with eight arguments. */
    inline Uintn operator()(EfiArg arg1, EfiArg arg2, EfiArg arg3, EfiArg arg4,
                            EfiArg arg5, EfiArg arg6, EfiArg arg7, EfiArg arg8) {
        Uintn rc;
        ASM ("subq $72, %%rsp\n"
             "mov %[arg3], %%r8\n"
             "mov %[arg4], %%r9\n"
             "mov %[arg5], %%rax\n"
             "mov %%rax, 32(%%rsp)\n"
             "mov %[arg6], %%rax\n"
             "mov %%rax, 40(%%rsp)\n"
             "mov %[arg7], %%rax\n"
             "mov %%rax, 48(%%rsp)\n"
             "mov %[arg8], %%rax\n"
             "mov %%rax, 56(%%rsp)\n"
             "call *%[func]\n"
             "addq $72, %%rsp\n"
             : "=&a"(rc)
             : [func]"r"(vm::mm->PhysToVirt(_addr)),
               "c"(arg1),
               "d"(arg2),
               [arg3]"m"(arg3),
               [arg4]"m"(arg4),
               [arg5]"m"(arg5),
               [arg6]"m"(arg6),
               [arg7]"m"(arg7),
               [arg8]"m"(arg8)
             : "r8", "r9", "r10", "r11"
             );
        return rc;
    };

private:
    vm::Paddr _addr;
};

} /* namespace efi */

#endif /* MD_EFI_H_ */
