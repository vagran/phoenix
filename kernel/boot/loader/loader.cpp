/*
 * /phoenix/kernel/boot/loader/loader.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/* Separate C++ file to make it possible to include sys.h */

#include <sys.h>

#include "loader.h"

void
__Fault(const char *file, int line, const char *msg, ...)
{
    LoaderPrint(L"Fault occurred in %a:%d:\n", file, line);
    va_list args;
    va_start(args, msg);
    WCHAR_T *wMsg = LoaderStrConvert(msg);
    for (WCHAR_T *p = wMsg; *p; p++) {
        if (*p == '%' && (p == wMsg || *(p - 1) != '%') && p[1] == 's') {
            p[1] = 'a';
        }
    }
    LoaderPrintV(wMsg, args);
    va_end(args);

    while(true) {
        __asm__ __volatile__ ("pause");
    }
}

int
LoadElfImage(Elf_File *file, Elf *elf, vaddr_t *entry_addr)
{
    if (elf_kind(elf) != ELF_K_ELF) {
        LoaderPrint(L"Invalid type of ELF binary: %d\n", elf_kind(elf));
        return -1;
    }

    Elf64_Ehdr *ehdr = elf64_getehdr(elf);
    if (!ehdr) {
        LoaderPrint(L"Failed to get ELF execution header: %a\n", elf_errmsg(-1));
        return -1;
    }

    Elf64_Phdr *phdr = elf64_getphdr(elf);
    Elf64_Half segmentIdx;
    for (segmentIdx = 0; segmentIdx < ehdr->e_phnum; segmentIdx++) {
        /* Load only segments of PT_LOAD type. */
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        u64 start_off = phdr->p_offset;

        vm::Paddr start_pa(phdr->p_paddr);
        u64 file_size = phdr->p_filesz;
        vm::Vaddr mem_size(phdr->p_memsz);

        if (phdr->p_align != 0 && phdr->p_align != 1 &&
            IsPowerOf2(phdr->p_align)) {

            u64 pad = start_off - RoundDown2(start_off, phdr->p_align);
            file_size += pad;
            start_off -= pad;
            pad = start_pa - RoundDown2<paddr_t>(start_pa, phdr->p_align);
            mem_size += pad;
            start_pa -= pad;
        }

        if (start_pa & (vm::PAGE_SIZE - 1)) {
            LoaderPrint(L"Unaligned executable binary segments loading "
                        "is not supported\n");
            return -1;
        }

        if (LoaderGetMemory(start_pa, mem_size.RoundUp().GetPageIdx())) {
            LoaderPrint(L"Failed to get memory for a segment: %d bytes at %x\n",
                        mem_size, start_pa);
            return -1;
        }

        if (LoaderReadFile(file, start_off, file_size, start_pa)) {
            LoaderPrint(L"Failed to load segment %d bytes at %x\n",
                        file_size, start_off);
            return -1;
        }
        vm::Vaddr phdr_addr(phdr);
        phdr_addr += ehdr->e_phentsize;
        phdr = phdr_addr;
    }

    if (entry_addr) {
        *entry_addr = ehdr->e_entry;
    }

    return 0;
}
