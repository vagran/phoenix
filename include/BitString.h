/*
 * /phoenix/include/BitString.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef BITSTRING_H_
#define BITSTRING_H_

/** @file BitString.h
 * Bit strings operations.
 *
 * Common operations for manipulating bits strings are defined in this file.
 * The implementation is machine-dependent since it must take into account the
 * machine endianness and utilize special CPU instructions, e.g. for searching
 * the first bit set, if such available.
 */

/** Class for manipulating bit strings. Bit string is a sequence of bits which
 * are indexed by a null-based index. Each bit in a string can be accessed
 * (checked or modified) individually by its index.
 * @param numBits Number of bits in a string. If this parameter is not zero
 *      then the string is allocated statically in the class. Otherwise the
 *      string is allocated by the caller and the bitmap and number of bits are
 *      provided to the constructor.
 */
template <size_t numBits = 0>
class BitString {
public:
    inline BitString() {
        memset(_bits, 0, sizeof(_bits));
    }

    /** Set bit at specified position.
     *
     * @param idx Null based bit index.
     */
    inline void Set(size_t idx) {
        ASSERT(idx < numBits);
        _bits[idx / NBBY] |= 1 << (idx % NBBY);
    }

    /** Clear bit at specified position.
     *
     * @param idx Null based bit index.
     */
    inline void Clear(size_t idx) {
        ASSERT(idx < numBits);
        _bits[idx / NBBY] &= ~(1 << (idx % NBBY));
    }

    /** Check if  bit is set at specified position.
     *
     * @param idx Null based bit index.
     * @return @a true if the bit is set, @a false otherwise.
     */
    inline bool IsSet(size_t idx) {
        ASSERT(idx < numBits);
        return _bits[idx / NBBY] & (1 << (idx % NBBY));
    }

    /** Check if  bit is clear at specified position.
     *
     * @param idx Null based bit index.
     * @return @a true if the bit is clear, @a false otherwise.
     */
    inline bool IsClear(size_t idx) {
        ASSERT(idx < numBits);
        return !IsSet(idx);
    }

    /** Check if  bit is set at specified position. Equivalent of @ref IsSet
     * method.
     *
     * @param idx Null based bit index.
     * @return @a true if the bit is set, @a false otherwise.
     */
    inline bool operator[](size_t idx) {
        return IsSet(idx);
    }

    /** Find first set bit.
     *
     * @return Index of first bit set. -1 if no bits set.
     */
    int FirstSet() {
        const size_t numWords = numBits / (sizeof(uintptr_t) * NBBY);
        for (size_t word = 0; word < numWords; word++) {
            uintptr_t x = static_cast<uintptr_t *>(static_cast<void *>(_bits))[word];
            if (x) {
                size_t bit = cpu::bsf(x);
                bit += word * sizeof(uintptr_t) * NBBY;
                return bit;
            }
        }
        /* Check remainder. */
        for (size_t idx = numWords * sizeof(uintptr_t) * NBBY;
             idx < numBits;
             idx++) {

            if (IsSet(idx)) {
                return idx;
            }
        }
        return -1;
    }

    /** Find first clear bit.
     *
     * @return Index of first bit set. -1 if no bits set.
     */
    int FirstClear() {
        const size_t numWords = numBits / (sizeof(uintptr_t) * NBBY);
        for (size_t word = 0; word < numWords; word++) {
            uintptr_t x = static_cast<uintptr_t *>(static_cast<void *>(_bits))[word];
            if (x != static_cast<uintptr_t>(~0)) {
                size_t bit = cpu::bsf(~x);
                bit += word * sizeof(uintptr_t) * NBBY;
                return bit;
            }
        }
        /* Check remainder. */
        for (size_t idx = numWords * sizeof(uintptr_t) * NBBY;
             idx < numBits;
             idx++) {

            if (IsClear(idx)) {
                return idx;
            }
        }
        return -1;
    }

    /** Clear all bits in the string. */
    inline void ClearAll() {
        memset(_bits, 0, sizeof(_bits));
    }

    inline void SetAll() {
        memset(_bits, 0xff, sizeof(_bits));
    }

    inline void Invert() {
        const size_t numWords = numBits / (sizeof(uintptr_t) * NBBY);
        for (size_t word = 0; word < numWords; word++) {
            static_cast<uintptr_t *>(static_cast<void *>(_bits))[word] ^= ~0ul;
        }
        /* Invert remainder. */
        for (size_t idx = numWords * sizeof(uintptr_t);
             idx < (numBits + NBBY - 1) / NBBY;
             idx++) {

            _bits[idx] ^= 0xff;
        }
    }

private:
    u8 _bits[(numBits + NBBY - 1) / NBBY];
};

/** Bit string class specialization for dynamically allocated bitmaps.
 */
template <>
class BitString<0> {
public:
    /** Construct bit string from dynamically allocated bitmaps.
     *
     * @param bitmap Pointer to the buffer where the bitmap is allocated. The
     *      size of buffer should be enough to hold all bytes occupied by the
     *      bitmap bits.
     * @param numBits Number of bits in the bitmap.
     */
    inline BitString(u8 *bitmap, const size_t numBits) :
        _bits(bitmap),
        _numBits(numBits),
        _bitmapSize((numBits + NBBY - 1) / NBBY)
    {
        memset(_bits, 0, _bitmapSize);
    }

    /** Set bit at specified position.
     *
     * @param idx Null based bit index.
     */
    inline void Set(size_t idx) {
        ASSERT(idx < _numBits);
        _bits[idx / NBBY] |= 1 << (idx % NBBY);
    }

    /** Clear bit at specified position.
     *
     * @param idx Null based bit index.
     */
    inline void Clear(size_t idx) {
        ASSERT(idx < _numBits);
        _bits[idx / NBBY] &= ~(1 << (idx % NBBY));
    }

    /** Check if  bit is set at specified position.
     *
     * @param idx Null based bit index.
     * @return @a true if the bit is set, @a false otherwise.
     */
    inline bool IsSet(size_t idx) {
        ASSERT(idx < _numBits);
        return _bits[idx / NBBY] & (1 << (idx % NBBY));
    }

    /** Check if  bit is clear at specified position.
     *
     * @param idx Null based bit index.
     * @return @a true if the bit is clear, @a false otherwise.
     */
    inline bool IsClear(size_t idx) {
        ASSERT(idx < _numBits);
        return !IsSet(idx);
    }

    /** Check if  bit is set at specified position. Equivalent of @ref IsSet
     * method.
     *
     * @param idx Null based bit index.
     * @return @a true if the bit is set, @a false otherwise.
     */
    inline bool operator[](size_t idx) {
        return IsSet(idx);
    }

    /** Find first set bit.
     *
     * @return Index of first bit set. -1 if no bits set.
     */
    int FirstSet() {
        const size_t numWords = _numBits / (sizeof(uintptr_t) * NBBY);
        for (size_t word = 0; word < numWords; word++) {
            uintptr_t x = static_cast<uintptr_t *>(static_cast<void *>(_bits))[word];
            if (x) {
                size_t bit = cpu::bsf(x);
                bit += word * sizeof(uintptr_t) * NBBY;
                return bit;
            }
        }
        /* Check remainder. */
        for (size_t idx = numWords * sizeof(uintptr_t) * NBBY;
             idx < _numBits;
             idx++) {

            if (IsSet(idx)) {
                return idx;
            }
        }
        return -1;
    }

    /** Find first clear bit.
     *
     * @return Index of first bit set. -1 if no bits set.
     */
    int FirstClear() {
        const size_t numWords = _numBits / (sizeof(uintptr_t) * NBBY);
        for (size_t word = 0; word < numWords; word++) {
            uintptr_t x = static_cast<uintptr_t *>(static_cast<void *>(_bits))[word];
            if (x != static_cast<uintptr_t>(~0)) {
                size_t bit = cpu::bsf(~x);
                bit += word * sizeof(uintptr_t) * NBBY;
                return bit;
            }
        }
        /* Check remainder. */
        for (size_t idx = numWords * sizeof(uintptr_t) * NBBY;
             idx < _numBits;
             idx++) {

            if (IsClear(idx)) {
                return idx;
            }
        }
        return -1;
    }

    /** Clear all bits in the string. */
    inline void ClearAll() {
        memset(_bits, 0, _bitmapSize);
    }

    inline void SetAll() {
        memset(_bits, 0xff, _bitmapSize);
    }

    inline void Invert() {
        const size_t numWords = _numBits / (sizeof(uintptr_t) * NBBY);
        for (size_t word = 0; word < numWords; word++) {
            static_cast<uintptr_t *>(static_cast<void *>(_bits))[word] ^= ~0ul;
        }
        /* Invert remainder. */
        for (size_t idx = numWords * sizeof(uintptr_t);
             idx < (_numBits + NBBY - 1) / NBBY;
             idx++) {

            _bits[idx] ^= 0xff;
        }
    }
private:
    u8 *_bits;
    const size_t _numBits, _bitmapSize;
};

#endif /* BITSTRING_H_ */
