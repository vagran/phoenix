/*
 * /phoenix/lib/common/BuddyAllocator.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file BuddyAllocator.cpp
 * Universal buddy allocator implementation.
 */

#include <sys.h>

BuddyAllocatorBase::BuddyAllocatorBase()
{

}

RetCode
BuddyAllocatorBase::Initialize(Addr startAddress, Addr endAddress, int minOrder,
                               int maxOrder, size_t cacheSize)
{
    if (startAddress >= endAddress) {
        return RC(INV_PARAM);
    }
    _startAddress = startAddress;
    _endAddress = endAddress;
    _minOrder = minOrder;

    /* Find maximal possible order. */
    int order = _GetOrder(endAddress - startAddress);
    Addr alignedStart, alignedEnd;
    Addr size;
    do {
        size = _GetOrderSize(order);
        alignedStart = RoundUp2(startAddress, size);
        alignedEnd = RoundDown2(endAddress, size);
        order--;
    } while (alignedEnd - alignedStart != size);
    order++;

    if (maxOrder == -1 || order < maxOrder) {
        _maxOrder = order;
    } else {
        if (maxOrder > static_cast<int>(MAX_ORDER)) {
            return RC(INV_PARAM);
        }
        _maxOrder = maxOrder;
    }

    /* Range should be aligned on blocks of maximal order. */
    if (startAddress != RoundUp2(startAddress, _GetOrderSize(_maxOrder)) ||
        endAddress != RoundUp2(endAddress, _GetOrderSize(_maxOrder))) {

        return RC(INV_PARAM);
    }

    if (cacheSize > MAX_CACHE_SIZE) {
        return RC(INV_PARAM);
    }

    //XXX

    _isInitialized = true;
    return RC(SUCCESS);
}
