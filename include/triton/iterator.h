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
     * @param endIterator Indicates that end iterator should be created, i.e.
     *      the one returned by @ref end function used in C++-style iterations.
     * @return Iterator object.
     */
    virtual
    Iterator<T>
    __iter__(bool endIterator = false) = 0;
};

template <typename T>
constexpr inline Iterable<T> &
object(Iterable<T> &obj)
{
    return obj;
}

template <typename T>
constexpr inline Iterable<T> &&
object(Iterable<T> &&obj)
{
    return static_cast<Iterable<T> &&>(obj);
}

/** Create iterator for a given iterable object. This function is used in
 * C++-style iterations, particularly in "range-based for" loop.
 * @param iterable Iterable object.
 * @return Iterator.
 */
template<typename T>
constexpr Iterator<T>
begin(Iterable<T> &iterable)
{
    return iterable.__iter__();
}

/** Create end iterator for a given iterable object. This function is used in
 * C++-style iterations, particularly in "range-based for" loop in pair with
 * @ref begin function.
 *
 * @param iterable Iterable object.
 * @return Iterator.
 */
template<typename T>
constexpr Iterator<T>
end(Iterable<T> &iterable)
{
    return iterable.__iter__(true);
}

/** Iterator implementation interface. All iterators for a specific iterable
 * classes should be derived from this class.
 */
template <typename T>
class IteratorImpl: public Object {
protected:
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
    /** Size of iterator implementation object. End-iterator if zero. */
    size_t _objSize = 0;
    /** Next value if available. */
    T *_nextValue = nullptr;

    void
    _GetNextValue()
    {
        ASSERT(_itPtr);
        if (!_itPtr->__hasNext__()) {
            return;
        }
        try {
            _nextValue = &_itPtr->__next__();
        } catch (StopIteration &) {
            _nextValue = nullptr;
        }
    }
public:
    inline
    Iterator() {}

    Iterator(const Iterator<T> &it)
    {
        _objSize = it._objSize;
        if (_objSize > sizeof(_implArea)) {
            _itPtr = it._itPtr;
        } else if (_objSize) {
            memcpy(_implArea, it._implArea, _objSize);
            _itPtr = reinterpret_cast<IteratorImpl<T> *>(_implArea);
        }
        _nextValue = it._nextValue;
    }

    Iterator(Iterator<T> &&it)
    {
        _objSize = it._objSize;
        if (_objSize > sizeof(_implArea)) {
            _itPtr = move(it._itPtr);
        } else if (_objSize) {
            memcpy(_implArea, it._implArea, _objSize);
            _itPtr = reinterpret_cast<IteratorImpl<T> *>(_implArea);
        }
        _nextValue = it._nextValue;
    }

    /** Assign iterator implementation object to iterator interface.
     *
     * @param args Arguments to @a IteratorImplType constructor.
     *
     * Usage example:
     * @code
     * virtual
     * Iterator<ValueType>
     * __iter__(bool endIterator = false)
     * {
     *     Iterator<ValueType> it;
     *     if (endIterator) {
     *         return it;
     *     }
     *     it.template Assign<ListIterator>(*this);
     *     return it;
     * }
     * @endcode
     */
    template <class IteratorImplType,
              class AllocatorT = Allocator<IteratorImplType>,
              typename... Args>
    Iterator<T> &
    Assign(Args&&... args)
    {
        typedef typename AllocatorT::template Rebind<IteratorImplType> IterAllocator;
        ASSERT(_objSize == 0);
        _objSize = sizeof(IteratorImplType) ;
        if (_objSize > sizeof(_implArea)) {
            _itPtr = IterAllocator().Allocate(forward<Args>(args)...);
        } else {
            _itPtr = *new(_implArea) IteratorImplType(forward<Args>(args)...);
        }
        _GetNextValue();
        return *this;
    }

    /* Operators for supporting range-based for loop. */
    bool
    operator !=(Iterator<T> &it)
    {
        Iterator<T> *masterIt; /* Non-end iterator. */
        if (!_objSize) {
            masterIt = &it;
            if (!it._objSize) {
                return false;
            }
        } else {
            masterIt = this;
            if (it._objSize) {
                return false;
            }
        }
        return masterIt->_nextValue != nullptr;
    }

    inline void
    operator ++()
    {
        _GetNextValue();
    }

    inline constexpr T &
    operator *()
    {
        return *_nextValue;
    }

    /** Implement @ref Iterable interface in order to enable iterating over an
     * iterator itself.
     */
    inline
    Iterator<T>
    __iter__(bool endIterator = false)
    {
        if (endIterator) {
            return Iterator<T>();
        }
        ASSERT(_itPtr);
        return *this;
    }

    /** Return the next item from the container. If there are no further items,
     * raise the @ref StopIteration exception.
     */
    T &
    __next__()
    {
        if (!_nextValue) {
            throw StopIteration();
        }
        T *retValue = _nextValue;
        _GetNextValue();
        return *retValue;
    }
};

} /* namespace triton */

#endif /* ITERATOR_H_ */
