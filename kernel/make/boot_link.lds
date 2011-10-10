/* /phoenix/kernel/make/boot_link.lds
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/*
 * This script is used for partial linking of all object files involved into the
 * kernel bootstrapping.
 */

SECTIONS {
    .boot : {
        *(.text .data .rodata*)
    }
    
    .bss : {
        *(.bss)
    }
}
