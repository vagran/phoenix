/*
 * /phoenix/kernel/sys/bitops.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef BITOPS_H_
#define BITOPS_H_

/** @file bitops.h
 * Bit operations.
 *
 * Common operations for manipulating bits strings are defined in this file.
 */

/** Set specified bit in a string */
#define BitSet(a, i)        (((u8 *)(a))[(i) / NBBY] |= 1 << ((i) % NBBY))
/** Clear specified bit in a string */
#define BitClear(a, i)      (((u8 *)(a))[(i) / NBBY] &= ~(1 << ((i) % NBBY)))
/** Check if specified bit is set in a string */
#define BitIsSet(a, i)      (((const u8 *)(a))[(i) / NBBY] & (1 << ((i) % NBBY)))
/** Check if specified bit is cleared in a string */
#define BitIsClear(a, i)     (!BitIsSet(a, i))

namespace {

/** Find the first set bit in a provided string.
 *
 * This function finds the first bit set in a provided bits string. Scanning for
 * a bit in a byte is done starting from least significant bits.
 * @param a String of bits.
 * @param numBits Total number of bits in a provided string.
 * @return The first set bit index, -1 if no bits set.
 */
inline int
BitFirstSet(void *a, u32 numBits)
{
    u32 numWords = (numBits + sizeof(u32) * NBBY - 1) / (sizeof(u32) * NBBY);
    for (u32 word = 0; word < numWords; word++) {
        u32 x = ((u32 *)a)[word];
        if (x) {
            int bit = bsf(x);
            bit += word * sizeof(u32) * NBBY;
            if (bit >= (int)numBits) {
                return -1;
            }
            return bit;
        }
    }
    return -1;
}

/** Find the first cleared bit in a provided string.
 *
 * This function finds the first bit cleared in a provided bits string. Scanning
 * for a bit in a byte is done starting from least significant bits.
 * @param a String of bits.
 * @param numBits Total number of bits in a provided string.
 * @return The first cleared bit index, -1 if no bits set.
 */
inline int
BitFirstClear(void *a, u32 numBits)
{
    u32 numWords = (numBits + sizeof(u32) * NBBY - 1) / (sizeof(u32) * NBBY) ;
    for (u32 word = 0; word < numWords; word++) {
        u32 x = ((u32 *)a)[word];
        if (x != (u32)~0) {
            int bit = bsf(~x);
            bit += word * sizeof(u32) * NBBY;
            if (bit >= (int)numBits) {
                return -1;
            }
            return bit;
        }
    }
    return -1;
}

} /* Anonymous namespace */

#endif /* BITOPS_H_ */
