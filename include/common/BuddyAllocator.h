/*
 * /phoenix/include/common/BuddyAllocator.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file BuddyAllocator.h
 * Universal buddy allocator.
 */

#ifndef BUDDYALLOCATOR_H_
#define BUDDYALLOCATOR_H_

/** Universal buddy allocator. The allocator is abstracted from underlying
 * resource type, i.e. it does not know what kind of resource is allocated –
 * is it physical or virtual memory, some ID space or anything else. It operates
 * with address ranges only. @n
 *
 * Note that the allocator is capable for allocations and freeings only – it
 * does not provide any way to associate some client meta-information with
 * allocated blocks. If such meta-information is required, the client should
 * implement some mapping between block address and its meta-information block
 * (e.g. using binary search trees). @n
 *
 * The buddy allocator is implemented using blocks bitmaps and free blocks
 * cache. When the allocator is initialized it allocates several internal
 * structures – a set of block bitmaps and a pool of cached free blocks. The set
 * of bitmaps is a map of currently free blocks. Each bitmap corresponds
 * to one order with which the allocator operates. Each set bit in a bitmap
 * corresponds to a free block of that order. @n
 *
 * Free blocks cache is a number of lists (one list for each order) and a tree
 * which store some amount of free blocks of each order. Tree is used because
 * when a block is freed, its buddy can be coalesced if it is free. In such case
 * it must be removed from free blocks cache of one order and placed to another
 * one. In case lists only are used without a tree for free blocks cache then
 * linear search should be used in order to find coalesced block and delete it
 * from cache. @n
 */
class BuddyAllocatorBase {
public:
    /** Address type for representing all ranges with which the allocator
     * operates.
     */
    typedef uintptr_t Addr;

    /** Various constants. */
    enum :size_t {
        /** Maximal allowed value for @a maxOrder parameter. */
        MAX_ORDER = sizeof(Addr) * NBBY,
        /** Maximal cache size in elements. */
        MAX_CACHE_SIZE = U16_MAX,
    };

    /** The allocator constructor. After the object is constructed it still is
     * not usable until @ref Initialize method is called.
     */
    BuddyAllocatorBase();

    ~BuddyAllocatorBase();

    /** Initialize the allocator. It will require some memory allocations for
     * the allocator internal data structures.
     *
     * @param startAddress Start address of the managed address range.
     * @param endAddress One-past-end address of the managed address range.
     * @param minOrder Minimal order of allocated blocks, i.e. minimal
     *      allocation size is (2 ^ @a minOrder) units.
     * @param maxOrder Maximal order of allocated blocks, i.e. maximal
     *      allocation size is (2 ^ @a maxOrder) units. Default value -1
     *      indicates that maximal order is limited by the provided range size.
     * @param cacheSize Size of the free blocks cache in elements. Should not
     *      exceed @ref MAX_CACHE_SIZE value.
     * @return @ref RetCode::SUCCESS if successfully initialized, error code
     *      otherwise.
     */
    RetCode Initialize(Addr startAddress, Addr endAddress, int minOrder = 0,
                       int maxOrder = -1, size_t cacheSize = 8192);

private:
    bool _isInitialized = false;
    Addr _startAddress, _endAddress;
    int _minOrder, _maxOrder;

    /** Free blocks cache entry. */
    class CacheEntry {
    public:
        typedef u16 Index; /**< This entry index in the cache table. */
        typedef Index ListHead; /**< Head of entries list. */

        /** Index which indicates cache entry null reference. */
        static const Index NONE = ~0;

        int Compare(CacheEntry &e) { return address - e.address; }
        int Compare(Addr &address) { return address - this->address; }
        typedef RBTree<CacheEntry, &CacheEntry::Compare, Addr, &CacheEntry::Compare> Tree;

        Addr address;  /**< Address of the block this entry represents. */
        Index next = NONE, /**< Next entry index when in list. */
              prev = NONE; /**< Previous entry index when in list. */
        Tree::Entry treeEntry;

        /** Get entry pointer by its index. */
        inline CacheEntry *GetPtr(CacheEntry *cache, Index idx) { return &cache[idx]; }
        /** Get this entry index. */
        inline Index GetIdx(CacheEntry *cache) { return this - cache; }

        /** Insert entry in a list as a head.
         *
         * @param head Head of the list to insert the entry to.
         */
        void Insert(CacheEntry *cache, ListHead &head);

        /** Delete entry from a list.
         *
         * @param head Head of the list to delete the entry from.
         */
        void Delete(CacheEntry *cache, ListHead &head);
    };

    /** Storage which holds the cache entries. */
    CacheEntry *_cache = 0;
    /** Number of elements in the cache. */
    size_t _cacheSize;
    /** Pool of cache entries. */
    CacheEntry::ListHead _freeCacheEntries = CacheEntry::NONE;
    /** Tree of cache entries. */
    CacheEntry::Tree _cacheTree;

    /** Each managed order is represented by one instance of this class. */
    class OrderPool {
    public:
        /** Initialize pool element.
         *
         * @param numBlocks Number of blocks of corresponding order.
         * @return Status code.
         */
        RetCode Initialize(size_t numBlocks);
        ~OrderPool();
    private:
        u8 *_bitmapData = 0; /**< Storage for bitmap data. */
        BitString<> _bitmap; /**< Free blocks bitmap. */
        CacheEntry::ListHead _freeBlocks; /** Free blocks cache. */
    };

    /** All managed resources are represented in this pool. */
    OrderPool *_pool = 0;

    /** Get order which corresponds to the provided size. */
    inline int _GetOrder(Addr size) {
        int order = 0;
        Addr osize = 1;
        while (osize < size) {
            order++;
            osize <<= 1;
        }
        return order;
    }

    /** Get size which corresponds to the provided order. */
    inline Addr _GetOrderSize(int order) {
        ASSERT(order >= 0);
        return 1 << order;
    }

    /** Release all allocated resources. */
    void _Free();
};

/** Universal buddy allocator.
 * @param AddrType Type of the address with which the allocator operates.
 */
template <class AddrType>
class BuddyAllocator : public BuddyAllocatorBase {
public:
    inline BuddyAllocator() : BuddyAllocatorBase() { }

    /** @see BuddyAllocatorBase::Initialize */
    inline RetCode Initialize(AddrType startAddress, AddrType endAddress,
                              int minOrder = 0,  int maxOrder = -1,
                              size_t cacheSize = 8192)
    {
        return BuddyAllocatorBase::Initialize(startAddress, endAddress,
                                              minOrder, maxOrder, cacheSize);
    }
};


#endif /* BUDDYALLOCATOR_H_ */
