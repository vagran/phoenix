/*
 * /phoenix/include/triton/iterator.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file iterator.h
 * Triton iterators interfaces description.
 */

#ifndef ITERATOR_H_
#define ITERATOR_H_

namespace triton {

/** Interface for objects which support iteration over their contained elements. */
template <typename T>
class Iterable {
public:
    /** Initialize iteration over iterable object elements.
     *
     * @return Iterator object.
     */
    virtual
    Iterator<T>
    __iter__() = 0;
};

/** Iterator implementation interface. All iterators for a specific iterable
 * classes should be derived from this class.
 */
template <typename T>
class IteratorImpl: public Object {
private:
    /** This member is a hint which indicates that @ref __next__ method still
     * can be called to return next item. It should be used to optimize
     * iteration in order to prevent from @ref StopIteration exception throwing
     * from @ref __next__ method. Either iterator constructor or @ref __next__
     * method should set it to @a true when there are no more items to iterate.
     */
    bool _hasNext = true;
public:
    /** Called by iterator wrapper to check if there can be more items to iterate. */
    inline bool
    __hasNext__()
    {
        return _hasNext;
    }

    /** Return the next item from the container. If there are no further items,
     * raise the @ref StopIteration exception. Should set @a _hasNext member
     * when last item is returned in order to optimize iteration performance.
     */
    virtual
    T &
    __next__() = 0;
};

/** Objects of this class are created by iterable classes. The object controls
 * iteration over elements stored in iterable object.
 */
template <typename T>
class Iterator: public Object, public Iterable<T> {
private:
    /** Pointer to iterator implementation object. */
    Ptr<IteratorImpl<T>> _itPtr;
    /** Internal constants. */
    enum {
        /** Number of bytes for area which is used to construct iterator
         * implementation object if it fits. If the object size is greater then
         * it is dynamically allocated by the provided allocator.
         */
        PREALLOCATED_SIZE = 128,
    };
    /** Iterator implementation objects preallocated space.
     * @see PREALLOCATED_SIZE
     */
    u8 _implArea[PREALLOCATED_SIZE];
public:
    inline
    Iterator() {}

    /** Assign iterator implementation object to iterator interface.
     *
     * @param args Arguments to @a IteratorImplType constructor.
     */
    template <class IteratorImplType,
              class AllocatorT = Allocator<IteratorImplType>,
              typename... Args>
    Iterator<T> &
    Assign(Args&&... args)
    {
        typedef typename AllocatorT::template Rebind<IteratorImplType> IterAllocator;
        if (sizeof(IteratorImplType) > sizeof(_implArea)) {
            _itPtr = IterAllocator().Allocate(forward<Args>(args)...);
        } else {
            _itPtr = *new(_implArea) IteratorImplType(forward<Args>(args)...);
        }
        return *this;
    }

    /* Operators for supporting range-based for loop. */
    inline bool operator !=(Iterator<T> &it) { return false; }
    inline void operator ++() {  }
    inline T &operator *() { return T(); }

    /** Implement @ref Iterable interface in order to enable iterating over an
     * iterator itself.
     */
    inline
    Iterator<T>
    __iter__()
    {
        return *this;
    }

    /** Return the next item from the container. If there are no further items,
     * raise the @ref StopIteration exception.
     */
    T &
    __next__()
    {

    }
};

} /* namespace triton */

#endif /* ITERATOR_H_ */
