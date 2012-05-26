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
    mutable int _refCount = 0;
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
    AddRef() const
    {
        _refCount++;
    }

    /** Release reference to the object.
     * @return Object reference counter after releasing the reference.
     */
    inline int
    Release() const
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

    /** Method for retrieving number of elements in an object if applicable.
     * @return Number of elements in this object.
     */
    virtual size_t
    __len__() const
    {
        return 0;
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

} /* namespace triton */

#endif /* OBJECT_H_ */
