/*
 * /phoenix/include/triton/allocator.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file allocator.h
 * Triton memory allocator class.
 */

#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

namespace triton {

/** Memory allocator for Triton classes. This class provides default
 * implementation. User-defined allocators should be derived from this class.
 * The allocator should throw throw @ref MemoryError exception in case of
 * memory exhaustion.
 */
template <typename T>
class Allocator {
private:
    template <typename Tother>
    struct rebind_impl {
        typedef Allocator<Tother> other;
    };

public:
    /** Helper for getting an allocator of same class with another type of
     * allocated objects (specified via @a Tother template parameter).
     *
     * Usage example:
     * @code
     * typedef Allocator<int> intAlloc;
     * typedef typename intAlloc::template Rebind<long> longAlloc;
     * @endcode
     */
    template <typename Tother>
    using Rebind = typename rebind_impl<Tother>::other;

    /** Allocate and construct one instance of object. This method should throw
     * @ref MemoryError exception if allocation fails.
     *
     * @param args Optional arguments for constructor.
     * @return Pointer to allocated object.
     */
    template<typename... Args>
    inline T *
    Allocate(Args&&... args)
    {
        return NEW T(forward<Args>(args)...);
    }

    /** Allocate and construct array of objects. This method should throw
     * @ref MemoryError exception if allocation fails.
     *
     * @param num Number of objects to allocate and construct.
     * @return Pointer to allocated objects array.
     */
    inline T *
    AllocateArray(size_t num)
    {
        return NEW T[num];
    }

    /** Destroy and free object allocated by @ref Allocate method.
     *
     * @param ptr Pointer to object to free.
     */
    inline void
    Free(T *ptr)
    {
        DELETE ptr;
    }

    /** Destroy and free array of objects allocated by @ref AllocateArray
     * method.
     *
     * @param ptr Pointer to array of objects to free.
     */
    inline void
    FreeArray(T *ptr)
    {
        DELETE[] ptr;
    }
};

} /* namespace triton */

#endif /* ALLOCATOR_H_ */
