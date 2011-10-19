/*
 * /phoenix/kernel/boot/loader/loader.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
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
/* Allocate physical memory of specified size at specified address. */
LOADER_EXTERN int LoaderGetMemory(vaddr_t address, u64 pages);
/* Read file chunk to the provided memory. */
LOADER_EXTERN int LoaderReadFile(Elf_File *file, u64 offset, u64 size, void *mem);

#endif /* LOADER_H_ */
