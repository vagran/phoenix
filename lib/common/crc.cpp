/*
 * /phoenix/lib/common/crc.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file crc.cpp
 * Implementation of cyclic redundancy check calculations.
 */

#include <sys.h>

Crc32::Crc32(u32 polynomial)
{
    _polynomial = polynomial;
    /* Build the CRC lookup table */
    for (int i = 0; i < 256; i++) {
        u32 crc = i;
        for (int j = 8 ; j > 0; j--) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ _polynomial;
            } else {
                crc >>= 1;
            }
        }
        _crcTable[i] = crc;
    }
}

u32
Crc32::Calculate(void *buf, size_t size, u32 crc)
{
    u8 *ptr = static_cast<u8 *>(buf);
    while (size) {
        crc = (crc >> 8) ^ _crcTable[(crc ^ *ptr) & 0xff];
        ptr++;
        size--;
    }
    return crc;
}
