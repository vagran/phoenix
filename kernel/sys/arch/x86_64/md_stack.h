/*
 * /phoenix/kernel/sys/arch/x86_64/md_stack.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file md_stack.h
 * Machine dependent functionality implementation for stack manipulations.
 */

#ifndef MD_STACK_H_
#define MD_STACK_H_

/** Pointer to function used to switch the stack to new location. Used with
 * @ref SwitchStack function.
 *
 * @param arg Optional argument to pass to the entry function.
 */
typedef void (*StackEntryFunc)(void *arg) __NORETURN;

/** Switch stack to new location. Control is immediately transferred to the
 * provided entry point. Returning from the new entry point is not possible so
 * the application should ensure that all necessary information is passed to the
 * new context.
 *
 * @param stackAddr Address of the new stack bottom. It is actually the address
 * past the last byte of available stack space. The stack will grow down from
 * this location.
 * @param entryFunc Function which should be called after the stack is switched.
 *      It is not possible to return from this function.
 * @param arg Argument to pass to entry function @a entryFunc.
 */
inline void
SwitchStack(vaddr_t stackAddr, StackEntryFunc entryFunc, void *arg = 0)
{
    ASM (
        "movq %[stackAddr], %%rsp\n"
        "jmp *%[entryFunc]\n"
        :
        : [arg]"D"(arg), [entryFunc]"a"(entryFunc), [stackAddr]"S"(stackAddr)
        : "memory"
        );
}

#endif /* MD_STACK_H_ */
