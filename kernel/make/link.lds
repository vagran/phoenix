/*
 * /phoenix/kernel/make/link.lds
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

ENTRY(Start)

SECTIONS {
    . = LOAD_ADDRESS;
    .boot : AT (LOAD_ADDRESS) {
        */boot.o(.boot)
        kernBootBss = ABSOLUTE(.);
        */boot.o(.bss)
    }
    kernBootEnd = ALIGN(0x1000);
    
    . = KERNEL_ADDRESS + kernBootEnd - LOAD_ADDRESS;
    .text : AT(kernBootEnd) {
        kernText = ABSOLUTE(.);
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    }
    
    . = ALIGN(0x1000);
    kernTextEnd = ABSOLUTE(.);
    
    /DISCARD/ : {
        */boot.o(.ctors .dtors)
    }
    
    .rodata : AT(kernTextEnd - kernText + kernBootEnd) {
        *(.rodata*)
        *(.gnu.linkonce.r.*)
        
        /* place global constructors and destructors list in .rodata section */
        . = ALIGN(8);
        _CTOR_LIST = .;
        QUAD((_CTOR_END - _CTOR_LIST) / 8 - 2)
        *(SORT(.ctors))
        QUAD(0)
        _CTOR_END = .;
        . = ALIGN(8);
        _DTOR_LIST = .;
        QUAD((_DTOR_END - _DTOR_LIST) / 8 - 2)
        *(SORT(.dtors))
        QUAD(0)
        _DTOR_END = .;
    }

    . = ALIGN(0x1000);
    kernRodataEnd = ABSOLUTE(.);
    
    /* Create RAM-disk from .ramdisk section of input object if exists */
    kernRamdisk = kernRodataEnd;
    .ramdisk : AT(kernRodataEnd - kernText + kernBootEnd) {
        *(.ramdisk)
    }
    
    . = ALIGN(0x1000);
    kernRamdiskEnd = ABSOLUTE(.);
    
    .data : AT(kernRamdiskEnd - kernText + kernBootEnd) {
        *(.data)
        kernRamdiskSize = ABSOLUTE(.);
        LONG(SIZEOF(.ramdisk))
    }
    
    . = ALIGN(0x1000);
    kernDataEnd = ABSOLUTE(.);
    
    .bss : AT(kernDataEnd - kernText + kernBootEnd) {
        *(.bss)
    }
    
    . = ALIGN(0x1000);
    kernEnd = ABSOLUTE(.);
    
    .stab : AT(kernEnd - kernText + kernBootEnd) {
        *(.stab)
    }
    
    . = ALIGN(0x1000);
    kernSymEnd = ABSOLUTE(.);
    
    .stabstr : AT(kernSymEnd - kernText + kernBootEnd) {
        *(.stabstr)
    }
    
    . = ALIGN(0x1000);
    kernStrtabEnd = ABSOLUTE(.);
    kernImageEnd = kernStrtabEnd;
    
    /DISCARD/ : {
        *(.comment)
    }
}
