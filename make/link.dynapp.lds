/*
 * /phoenix/make/link.dynapp.lds
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

ENTRY(AppStart)

PHDRS
{
    headers PT_PHDR FILEHDR PHDRS ;
    interp PT_INTERP ;
    text PT_LOAD FLAGS (5) ;
    rodata PT_LOAD FLAGS (4) ;
    data PT_LOAD FLAGS (6) ;
    dynamic PT_DYNAMIC ;
}

SECTIONS {    
    . = LOAD_ADDRESS;
    .text . : {
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    } :text
    
    . = ALIGN(4);
    .init . : {
        *(.init)
        *(.init.*)
    } :text
    
    . = ALIGN(4);
    .fini . : {
        *(.fini)
        *(.fini.*)
    } :text
    
    . = ALIGN(4);
    .plt . : {
        *(.plt)
    } :text
    
    . = ALIGN(0x1000);
    _etext = ABSOLUTE(.);
    
    .rodata . : {
        *(.rodata*)
        *(.gnu.linkonce.r.*)
        
        /* place global constructors and destructors list in .rodata section */
        . = ALIGN(4);
        _CTOR_LIST = .;
        LONG((_CTOR_END - _CTOR_LIST) / 4 - 2)
        *(SORT(.ctors))
        LONG(0)
        _CTOR_END = .;
        . = ALIGN(4);
        _DTOR_LIST = .;
        LONG((_DTOR_END - _DTOR_LIST) / 4 - 2)
        *(SORT(.dtors))
        LONG(0)
        _DTOR_END = .;
    } :rodata
    
    .interp . : { *(.interp) } :rodata :interp
    
    . = ALIGN(4);
    .dynamic . : { *(.dynamic) } :dynamic :rodata
    
    . = ALIGN(4);
    .hash . : { *(.hash) } :rodata
    
    . = ALIGN(4);
    .dynsym . : { *(.dynsym) } :rodata
    
    . = ALIGN(4);
    .dynstr . : { *(.dynstr) } :rodata
    
    . = ALIGN(4);
    .rel.plt . : { *(.rel.plt) } :rodata

    . = ALIGN(0x1000);
    _erodata = ABSOLUTE(.);
    
    .data . : {
        *(.data)
    } :data
    
    . = ALIGN(4);
    .got.plt . : {
        *(.got.plt)
    } :data
    
    . = ALIGN(0x1000);
    _edata = ABSOLUTE(.);
    
    .bss . : {
        *(.bss)
    } :data
    
    . = ALIGN(0x1000);
    _end = ABSOLUTE(.);
    
    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
