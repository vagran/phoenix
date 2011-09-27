OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    . = 0;
    ImageBase = .;
    
    .hash : {*(.hash)} /* this MUST come first! */
    
    . = ALIGN(4096);
    .eh_frame :
    {
        *(.eh_frame)
    }
    
    . = ALIGN(4096);
    .text :
    {
        *(.text)
    }
    
    .reloc :
    {
        *(.reloc)
    }
    
    . = ALIGN(4096);
    .data :
    {
        *(.rodata*)
        *(.got.plt)
        *(.got)
        *(.data*)
        *(.sdata)
        /* the EFI loader doesn't seem to like a .bss section, so we stick
         it all into .data: */
        *(.sbss)
        *(.scommon)
        *(.dynbss)
        *(.bss)
        *(COMMON)
        *(.rel.local)
        
        /* Place global constructors and destructors list in .data section */
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
    }
    
    . = ALIGN(4096);
    .dynamic : {*(.dynamic)}
    
    . = ALIGN(4096);
    .rela :
    {
        *(.rela.data*)
        *(.rela.got)
        *(.rela.stab)
    }
    
    . = ALIGN(4096);
    .dynsym : {*(.dynsym)}
    . = ALIGN(4096);
    
    .dynstr : {*(.dynstr)}
    . = ALIGN(4096);
    
    .ignored.reloc :
    {
        *(.rela.reloc)
    }
}
