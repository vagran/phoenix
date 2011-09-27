/*
 * libelf.h - public header file for libelf.
 * Copyright (C) 1995 - 2008 Michael Riepe
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* @(#) $Id: libelf.h 172 2010-12-13 07:54:26Z vagran $ */

#ifndef _LIBELF_H
#define _LIBELF_H

#include <types.h>

#if __LIBELF_INTERNAL__
#include "sys_elf.h"
#else /* __LIBELF_INTERNAL__ */
#include <sys_elf.h>
#endif /* __LIBELF_INTERNAL__ */

#if defined __GNUC__ && !defined __cplusplus
#define DEPRECATED  __attribute__((deprecated))
#else
#define DEPRECATED  /* nothing */
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Commands
 */
typedef enum {
    ELF_C_NULL = 0, /* must be first, 0 */
    ELF_C_READ,
    ELF_C_WRITE,
    ELF_C_CLR,
    ELF_C_SET,
    ELF_C_FDDONE,
    ELF_C_FDREAD,
    ELF_C_RDWR,
    ELF_C_NUM       /* must be last */
} Elf_Cmd;

/*
 * Flags
 */
#define ELF_F_DIRTY 0x1
#define ELF_F_LAYOUT    0x4
/*
 * Allow sections to overlap when ELF_F_LAYOUT is in effect.
 * Note that this flag is NOT portable, and that it may render
 * the output file unusable. Use with extreme caution!
 */
#define ELF_F_LAYOUT_OVERLAP    0x10000000

/*
 * File types
 */
typedef enum {
    ELF_K_NONE = 0, /* must be first, 0 */
    ELF_K_AR,
    ELF_K_COFF,
    ELF_K_ELF,
    ELF_K_NUM       /* must be last */
} Elf_Kind;

/*
 * Data types
 */
typedef enum {
    ELF_T_BYTE = 0, /* must be first, 0 */
    ELF_T_ADDR,
    ELF_T_DYN,
    ELF_T_EHDR,
    ELF_T_HALF,
    ELF_T_OFF,
    ELF_T_PHDR,
    ELF_T_RELA,
    ELF_T_REL,
    ELF_T_SHDR,
    ELF_T_SWORD,
    ELF_T_SYM,
    ELF_T_WORD,
    /*
     * New stuff for 64-bit.
     *
     * Most implementations add ELF_T_SXWORD after ELF_T_SWORD
     * which breaks binary compatibility with earlier versions.
     * If this causes problems for you, contact me.
     */
    ELF_T_SXWORD,
    ELF_T_XWORD,
    /*
     * Symbol versioning.  Sun broke binary compatibility (again!),
     * but I won't.
     */
    ELF_T_VDEF,
    ELF_T_VNEED,
    ELF_T_NUM       /* must be last */
} Elf_Type;

/*
 * Elf descriptor
 */
typedef struct Elf_s    Elf;

/*
 * Section descriptor
 */
typedef struct Elf_Scn_s    Elf_Scn;

/*
 * Archive member header
 */
typedef struct {
    char*           ar_name;
    time_t          ar_date;
    long            ar_uid;
    long            ar_gid;
    unsigned long   ar_mode;
    off_t           ar_size;
    char*           ar_rawname;
} Elf_Arhdr;

/*
 * Archive symbol table
 */
typedef struct {
    char*           as_name;
    size_t          as_off;
    unsigned long   as_hash;
} Elf_Arsym;

/*
 * Data descriptor
 */
typedef struct {
    void*       d_buf;
    Elf_Type    d_type;
    size_t      d_size;
    off_t       d_off;
    size_t      d_align;
    unsigned    d_version;
} Elf_Data;

/* Portable memory interface */

/* Allocate memory block. */
typedef void *(*Elf_Malloc)(size_t size);
/* Reallocate memory block. */
typedef void *(*Elf_Mrealloc)(void *ptr, size_t size);
/* Free memory block. */
typedef void (*Elf_Mfree)(void *ptr);

extern Elf_Malloc       elf_malloc;
extern Elf_Mrealloc     elf_mrealloc;
extern Elf_Mfree        elf_mfree;

/* Portable file interface */

struct Elf_File_s;
typedef struct Elf_File_s Elf_File;

/* 'whence' parameter for Elf_Seek function */
typedef enum {
    ELF_SEEK_SET,
    ELF_SEEK_END,
    ELF_SEEK_CUR
} Elf_Seek_Whence;

/* Seek to new current position. */
typedef size_t (*Elf_Seek)(Elf_File *file, size_t offset, Elf_Seek_Whence whence);
/* Read file data. */
typedef size_t (*Elf_Read)(Elf_File *file, char *buffer, size_t len);
/* Write file data. */
typedef size_t (*Elf_Write)(Elf_File *file, char *buffer, size_t len);
/* Close file. All occupied memory should be freed. */
typedef void (*Elf_Close)(Elf_File *file);

/*
 * Portable file object.
 */
struct Elf_File_s {
    void *f_priv; /* Private data for underlying implementation. */
    /* Public methods. */
    Elf_Seek Seek;
    Elf_Read Read;
    Elf_Write Write;
    Elf_Close Close;
};

/*
 * Function declarations
 */
extern Elf *elf_begin(Elf_File *fd, Elf_Cmd __cmd, Elf *__ref);
extern Elf *elf_memory(char *__image, size_t __size);
extern int elf_cntl(Elf *__elf, Elf_Cmd __cmd);
extern int elf_end(Elf *__elf);
extern const char *elf_errmsg(int __err);
extern int elf_errno(void);
extern void elf_fill(int __fill);
extern unsigned elf_flagdata(Elf_Data *__data, Elf_Cmd __cmd, unsigned __flags);
extern unsigned elf_flagehdr(Elf *__elf, Elf_Cmd __cmd, unsigned __flags);
extern unsigned elf_flagelf(Elf *__elf, Elf_Cmd __cmd, unsigned __flags);
extern unsigned elf_flagphdr(Elf *__elf, Elf_Cmd __cmd, unsigned __flags);
extern unsigned elf_flagscn(Elf_Scn *__scn, Elf_Cmd __cmd, unsigned __flags);
extern unsigned elf_flagshdr(Elf_Scn *__scn, Elf_Cmd __cmd, unsigned __flags);
extern size_t elf32_fsize(Elf_Type __type, size_t __count, unsigned __ver);
extern Elf_Arhdr *elf_getarhdr(Elf *__elf);
extern Elf_Arsym *elf_getarsym(Elf *__elf, size_t *__ptr);
extern off_t elf_getbase(Elf *__elf);
extern Elf_Data *elf_getdata(Elf_Scn *__scn, Elf_Data *__data);
extern Elf32_Ehdr *elf32_getehdr(Elf *__elf);
extern char *elf_getident(Elf *__elf, size_t *__ptr);
extern Elf32_Phdr *elf32_getphdr(Elf *__elf);
extern Elf_Scn *elf_getscn(Elf *__elf, size_t __index);
extern Elf32_Shdr *elf32_getshdr(Elf_Scn *__scn);
extern unsigned long elf_hash(const unsigned char *__name);
extern Elf_Kind elf_kind(Elf *__elf);
extern size_t elf_ndxscn(Elf_Scn *__scn);
extern Elf_Data *elf_newdata(Elf_Scn *__scn);
extern Elf32_Ehdr *elf32_newehdr(Elf *__elf);
extern Elf32_Phdr *elf32_newphdr(Elf *__elf, size_t __count);
extern Elf_Scn *elf_newscn(Elf *__elf);
extern Elf_Cmd elf_next(Elf *__elf);
extern Elf_Scn *elf_nextscn(Elf *__elf, Elf_Scn *__scn);
extern size_t elf_rand(Elf *__elf, size_t __offset);
extern Elf_Data *elf_rawdata(Elf_Scn *__scn, Elf_Data *__data);
extern char *elf_rawfile(Elf *__elf, size_t *__ptr);
extern char *elf_strptr(Elf *__elf, size_t __section, size_t __offset);
extern off_t elf_update(Elf *__elf, Elf_Cmd __cmd);
extern unsigned elf_version(unsigned __ver);
extern Elf_Data *elf32_xlatetof(Elf_Data *__dst, const Elf_Data *__src, unsigned __encode);
extern Elf_Data *elf32_xlatetom(Elf_Data *__dst, const Elf_Data *__src, unsigned __encode);

/*
 * Additional functions found on Solaris
 */
extern long elf32_checksum(Elf *__elf);

#if __LIBELF64
/*
 * 64-bit ELF functions
 * Not available on all platforms
 */
extern Elf64_Ehdr *elf64_getehdr(Elf *__elf);
extern Elf64_Ehdr *elf64_newehdr(Elf *__elf);
extern Elf64_Phdr *elf64_getphdr(Elf *__elf);
extern Elf64_Phdr *elf64_newphdr(Elf *__elf, size_t __count);
extern Elf64_Shdr *elf64_getshdr(Elf_Scn *__scn);
extern size_t elf64_fsize(Elf_Type __type, size_t __count, unsigned __ver);
extern Elf_Data *elf64_xlatetof(Elf_Data *__dst, const Elf_Data *__src, unsigned __encode);
extern Elf_Data *elf64_xlatetom(Elf_Data *__dst, const Elf_Data *__src, unsigned __encode);

/*
 * Additional functions found on Solaris
 */
extern long elf64_checksum(Elf *__elf);

#endif /* __LIBELF64 */

/*
 * ELF format extensions
 *
 * Return -1 on failure, 0 on success.
 */
extern int elf_getphdrnum(Elf *__elf, size_t *__resultp);
extern int elf_getshdrnum(Elf *__elf, size_t *__resultp);
extern int elf_getshdrstrndx(Elf *__elf, size_t *__resultp);

/*
 * Convenience functions
 *
 * elfx_update_shstrndx is elf_getshstrndx's counterpart.
 * It should be used to set the e_shstrndx member.
 * There is no update function for e_shnum or e_phnum
 * because libelf handles them internally.
 */
extern int elfx_update_shstrndx(Elf *__elf, size_t __index);

/*
 * Experimental extensions:
 *
 * elfx_movscn() moves section `__scn' directly after section `__after'.
 * elfx_remscn() removes section `__scn'.  Both functions update
 * the section indices; elfx_remscn() also adjusts the ELF header's
 * e_shnum member.  The application is responsible for updating other
 * data (in particular, e_shstrndx and the section headers' sh_link and
 * sh_info members).
 *
 * elfx_movscn() returns the new index of the moved section.
 * elfx_remscn() returns the original index of the removed section.
 * A return value of zero indicates an error.
 */
extern size_t elfx_movscn(Elf *__elf, Elf_Scn *__scn, Elf_Scn *__after);
extern size_t elfx_remscn(Elf *__elf, Elf_Scn *__scn);

/*
 * elf_delscn() is obsolete.  Please use elfx_remscn() instead.
 */
extern size_t elf_delscn(Elf *__elf, Elf_Scn *__scn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LIBELF_H */
