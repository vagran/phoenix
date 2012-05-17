/*
 * /phoenix/include/triton/object.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file object.h
 * This file describes Triton base class for all objects.
 */

#ifndef OBJECT_H_
#define OBJECT_H_

namespace triton {

class Object {
private:
    int _refCount = 0;
public:

    Object() {}

    virtual
    ~Object()
    {
        /* Should be destructed only when last reference released. */
        ASSERT(!_refCount);
    }

    /** Add reference to the object. */
    inline void
    AddRef()
    {
        _refCount++;
    }

    /** Release reference to the object.
     * @return Object reference counter after releasing the reference.
     */
    inline int
    Release()
    {
        ASSERT(_refCount);
        return --_refCount;
    }

    /** Method for retrieving name of object class.
     *
     * @return Name of this object class.
     */
    virtual const char *
    __name__() const
    {
        return "Object";
    }

    /** Type of object hash value. */
    typedef u64 hash_t;

    /** Get objects hash. The hash is used for optimized organizing of objects
     * in collections. The hash should provide good distribution of values. Two
     * equal objects must return the same hash value.
     *
     * @return Hash value of an object.
     */
    virtual hash_t
    __hash__() const
    {
        /* Default implementation returns constant value. */
        return 1;
    }
};

/** This built-in function should return Triton object from provided argument.
 * All classes provides this function which constructs class object from any
 * suitable arguments.
 *
 * Particularly this instance is overloaded for Triton object as argument which
 * is just returned back.
 *
 * @param obj Triton object.
 * @return The object passed in the argument.
 */
inline Object &
object(Object &obj)
{
    return obj;
}

inline Object &&
object(Object &&obj)
{
    return static_cast<Object &&>(obj);
}

class Container: public Object {
public:
    /** Method for retrieving number of elements in a container object.
     * @return Number of elements in this container object.
     */
    virtual size_t
    __len__()
    {
        return 0;
    }
};

template <typename T>
class Sequence: public Container {
public:
    /** Type of index of value in a sequence. */
    typedef size_t index_t;

    /** Get minimal value in a sequence. @ref ValueError is raised if the
     * sequence is empty.
     * @return Minimal value.
     */
    virtual T &
    __min__() = 0;

    /** Get maximal value in a sequence. @ref ValueError is raised if the
     * sequence is empty.
     * @return Maximal value.
     */
    virtual T &
    __max__() = 0;

    /** Get index of the first occurrence of @a value in the sequence. @ref
     * ValueError is raised if @a value is not present in a sequence.
     *
     * @param value Value to search.
     * @return Index of the first occurrence of @a value.
     */
    virtual index_t
    index(T &&value) = 0;

    /** Get total number of occurrences of @a value in the sequence.
     *
     * @param value Value to search.
     * @return Total number of occurrences of @a value.
     */
    virtual size_t
    count(T &&value) = 0;
};

class Iterable {
    //XXX
};

} /* namespace triton */

#endif /* OBJECT_H_ */
