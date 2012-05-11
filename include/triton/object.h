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

/** Base class for all Triton objects. All the library and client code objects
 * should be derived from this class.
 */
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

/** Get hash value of an object
 *
 * @param obj Container object.
 * @return Number of elements currently stored in container object.
 */
inline Object::hash_t
hash(Object &obj)
{
    return obj.__hash__();
}

inline Object::hash_t
hash(Object &&obj)
{
    return obj.__hash__();
}

/** Base class for all container classes. When Triton objects are stored in
 * containers, @ref Ptr class should be used for that.
 */
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

/** Get number of elements in container object.
 *
 * @param obj Container object.
 * @return Number of elements currently stored in container object.
 */
inline size_t
len(Container &obj)
{
    return obj.__len__();
}

/** Get number of elements in container object.
 *
 * @param obj Container object.
 * @return Number of elements currently stored in container object.
 */
inline size_t
len(Container &&obj)
{
    return obj.__len__();
}

/** Base class for all iterable classes - i.e. containers which support their
 * elements iteration.
 */
class Iterable: public Container {
    //XXX
};

} /* namespace triton */

#endif /* OBJECT_H_ */
