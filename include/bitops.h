/*
 * /phoenix/include/bitops.h
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

/* XXX must be rewritten to inline functions without C casts! */
/** Set specified bit in a string */
#define BIT_SET(a, i)       (((u8 *)/* XXX */(a))[(i) / NBBY] |= 1 << ((i) % NBBY))
/** Clear specified bit in a string */
#define BIT_CLEAR(a, i)     (((u8 *)/* XXX */(a))[(i) / NBBY] &= ~(1 << ((i) % NBBY)))
/** Check if specified bit is set in a string */
#define BIT_IS_SET(a, i)    (((const u8 *)/* XXX */(a))[(i) / NBBY] & (1 << ((i) % NBBY)))
/** Check if specified bit is cleared in a string */
#define BIT_IS_CLEAR(a, i)  (!BIT_IS_SET(a, i))

namespace {

/** Find the first set bit in a provided string.
 *
 * This function finds the first bit set in a provided bits string. Scanning for
 * a bit in a byte is done starting from least significant bits.
 * @param a String of bits.
 * @param numBits Total number of bits in a provided string. Should be multiple
 *      of sizeof(uintptr_t) * NBBY.
 * @return The first set bit index, -1 if no bits set.
 */
inline int
BitFirstSet(void *a, size_t numBits)
{
    ASSERT(numBits % (sizeof(uintptr_t) * NBBY) == 0);

    size_t numWords = (numBits + sizeof(uintptr_t) * NBBY - 1) /
                      (sizeof(uintptr_t) * NBBY);

    for (size_t word = 0; word < numWords; word++) {
        uintptr_t x = static_cast<uintptr_t *>(a)[word];
        if (x) {
            size_t bit = cpu::bsf(x);
            bit += word * sizeof(uintptr_t) * NBBY;
            if (bit >= numBits) {
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
 * @param numBits Total number of bits in a provided string. Should be multiple
 *      of sizeof(uintptr_t) * NBBY.
 * @return The first cleared bit index, -1 if no bits set.
 */
inline int
BitFirstClear(void *a, u32 numBits)
{
    ASSERT(numBits % (sizeof(uintptr_t) * NBBY) == 0);

    u32 numWords = (numBits + sizeof(uintptr_t) * NBBY - 1) /
                   (sizeof(uintptr_t) * NBBY) ;
    for (size_t word = 0; word < numWords; word++) {
        uintptr_t x = static_cast<uintptr_t *>(a)[word];
        if (x != static_cast<uintptr_t>(~0)) {
            size_t bit = cpu::bsf(~x);
            bit += word * sizeof(uintptr_t) * NBBY;
            if (bit >= numBits) {
                return -1;
            }
            return bit;
        }
    }
    return -1;
}

} /* Anonymous namespace */

#endif /* BITOPS_H_ */
