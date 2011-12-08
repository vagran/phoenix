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

BuddyAllocatorBase::~BuddyAllocatorBase()
{
    _Free();
}

void
BuddyAllocatorBase::_Free()
{
    DELETE [] _pool;
    _pool = 0;
    DELETE [] _cache;
    _cache = 0;
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
    _cacheSize = cacheSize;

    //int numOrders = maxOrder - minOrder + 1;

    /* Allocate cache. */
    _cache = NEW CacheEntry[_cacheSize];
    if (!_cache) {
        return RC(NO_MEMORY);
    }
    /* Place all cache entries in free list. */
    for (size_t idx = 0; idx < _cacheSize; idx++) {
        _cache[idx].Insert(_cache, _freeCacheEntries);
    }

    _isInitialized = true;
    return RC(SUCCESS);
}

void
BuddyAllocatorBase::CacheEntry::Insert(CacheEntry *cache, ListHead &head)
{
    next = head;
    prev = NONE;
    head = GetIdx(cache);
    if (next != NONE) {
        /* Non-empty list. */
        CacheEntry *nextEntry = GetPtr(cache, next);
        ASSERT(nextEntry->prev == NONE);
        nextEntry->prev = head;
    }
}

void
BuddyAllocatorBase::CacheEntry::Delete(CacheEntry *cache, ListHead &head)
{
    Index idx = GetIdx(cache);
    if (head != idx) {
        head = next;

        ASSERT(prev != NONE);
        CacheEntry *prevEntry = GetPtr(cache, prev);
        ASSERT(prevEntry->next == idx);
        prevEntry->next = next;
    } else {
        ASSERT(prev == NONE);
        head = next;
    }
    if (next != NONE) {
        CacheEntry *nextEntry = GetPtr(cache, next);
        nextEntry->prev = prev;
    }
}
