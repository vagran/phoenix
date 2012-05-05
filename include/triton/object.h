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
    /** Add reference to the object. */
    inline void AddRef() {
        _refCount++;
    }

    /** Release reference to the object. For dynamically allocated objects
     * last released reference causes the object deletion.
     */
    inline void Release() {
        ASSERT(_refCount);
        if (!--_refCount) {
            delete this;
        }
    }
};

} /* namespace triton */

#endif /* OBJECT_H_ */
