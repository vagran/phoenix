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

/** All Triton provided entities are defined in this namespace. */
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
};

/** Base class for all container classes. */
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

/** Base class for all iterable classes - i.e. containers which support their
 * elements iteration.
 */
class Iterable: public Container {
    //XXX
};

} /* namespace triton */

#endif /* OBJECT_H_ */
