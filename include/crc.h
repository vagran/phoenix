/*
 * /phoenix/include/crc.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file crc.h
 * Various algorithms for cyclic redundancy check implementation.
 */

#ifndef CRC_H_
#define CRC_H_

/** CCITT CRC32 implementation.
 *
 */
class Crc32 {
public:
    /** Construct CRC calculator.
     *
     * @param polynomial Polynomial value to use in calculations.
     */
    Crc32(u32 polynomial = 0x04c11db7);
    /** Calculate CRC for the buffer content. Can be used incrementally.
     *
     * @param buf Buffer with the data.
     * @param size Size of the buffer in bytes;
     * @param crc Initial value of CRC for the first call or previous returned
     *      value for the incremental call.
     * @return CRC value for the data in the provided buffer.
     */
    u32 Calculate(void *buf, size_t size, u32 crc = 0xffffffff);
private:
    u32 _polynomial;
    u32 _crcTable[256];
};


#endif /* CRC_H_ */
