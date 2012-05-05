/*
 * /phoenix/kernel/boot/loader/loader.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef LOADER_H_
#define LOADER_H_

#include <elf/libelf.h>

#ifdef __cplusplus
#define LOADER_EXTERN   extern "C"
#define WCHAR_T         wchar_t
#else
#define LOADER_EXTERN
#define WCHAR_T         CHAR16
#endif

/* Load ELF binary into memory. Return 0 on success. */
LOADER_EXTERN int LoadElfImage(Elf_File *file, Elf *elf, vaddr_t *entry_addr);
LOADER_EXTERN void LoaderPrint(const WCHAR_T *fmt, ...);
LOADER_EXTERN void LoaderPrintV(const WCHAR_T *fmt, va_list args);
LOADER_EXTERN WCHAR_T *LoaderStrConvert(const char *str);
/* Allocate physical memory of specified size at specified address. */
LOADER_EXTERN int LoaderGetMemory(vaddr_t address, u64 pages);
/* Allocate virtual memory block of specified size. */
LOADER_EXTERN void *LoaderAlloc(size_t size);
/* Free memory block allocated by LoaderAlloc function. */
LOADER_EXTERN void LoaderFree(void *ptr);
/* Read file chunk to the provided memory. */
LOADER_EXTERN int LoaderReadFile(Elf_File *file, u64 offset, u64 size, void *mem);

#endif /* LOADER_H_ */
