/*
 * /phoenix/include/triton/ptr.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file ptr.h
 * Triton objects pointers definition.
 */

#ifndef PTR_H_
#define PTR_H_

namespace triton {

/** Class for storing pointers to Triton objects. Whenever a pointer to an
 * object is created, passed or stored it should be wrapped into this class.
 */
template <class T>
class Ptr {
private:
    /** Pointer to referenced object. */
    T *_obj;
    /** Indicates that referenced object can be deleted by this class. Only
     * pointers constructed by pointer to an object can be deleted.
     */
    bool _canDelete;

    /** Release reference to an object. */
    inline void
    _ReleaseObj()
    {
        if (_obj && !_obj->Release() && _canDelete) {
            delete _obj;
        }
        _obj = nullptr;
    }
public:

    /** Construct null-pointer. */
    inline
    Ptr() {
        _obj = nullptr;
    }

    /** Construct pointer by pointer to an object. The object will be
     * automatically deleted when last reference is released.
     *
     * @param objPtr Pointer to a referenced object.
     */
    inline
    Ptr(T *objPtr) {
        if (objPtr) {
            objPtr->AddRef();
        }
        _obj = objPtr;
        _canDelete = true;
    }

    /** Construct pointer by a reference to an object. The object will not be
     * automatically deleted upon last reference releasing but still will have
     * proper reference counter kept.
     *
     * @param obj Reference to an object.
     */
    inline
    Ptr(T &obj) {
        obj.AddRef();
        _obj = &obj;
        _canDelete = false;
    }

    /** Construct pointer from another pointer.
     *
     * @param ptr Another pointer to get object from.
     */
    inline
    Ptr(Ptr<T> &ptr) {
        if (ptr._obj) {
            ptr._obj->AddRef();
            _obj = ptr._obj;
            _canDelete = ptr._canDelete;
        } else {
            _obj = nullptr;
        }
    }

    /** Move object reference from another pointer to a newly constructed one.
     *
     * @param ptr Pointer to move object reference from.
     */
    inline
    Ptr(Ptr<T> &&ptr) {
        _obj = ptr._obj;
        _canDelete = ptr._canDelete;
        ptr._obj = nullptr;
    }

    /** Destruct pointer and release reference to an object. Referenced object
     * is deleted if last reference was released and object can be deleted.
     */
    inline
    ~Ptr() {
        _ReleaseObj();
    }

    /** Assignment operator.
     *
     * @param ptr Pointer to assign object reference from.
     * @return Reference to itself.
     */
    inline Ptr<T> &
    operator =(Ptr<T> &ptr)
    {
        _ReleaseObj();
        if (ptr._obj) {
            ptr._obj->AddRef();
            _obj = ptr._obj;
            _canDelete = ptr._canDelete;
        } else {
            _obj = nullptr;
        }
        return *this;
    }

    /** Assignment operator with moving.
     *
     * @param ptr Pointer to move object reference from.
     * @return Reference to itself.
     */
    inline Ptr<T> &
    operator =(Ptr<T> &&ptr)
    {
        _ReleaseObj();
        _obj = ptr._obj;
        _canDelete = ptr._canDelete;
        ptr._obj = nullptr;
        return *this;
    }

    /** Assignment operator for object pointer assignment. */
    inline Ptr<T> &
    operator =(T *objPtr)
    {
        _ReleaseObj();
        if (objPtr) {
            objPtr->AddRef();
        }
        _obj = objPtr;
        _canDelete = true;
        return *this;
    }

    /** Assignment operator for object reference assignment. */
    inline Ptr<T> &
    operator =(T &obj)
    {
        _ReleaseObj();
        obj.AddRef();
        _obj = &obj;
        _canDelete = false;
        return *this;
    }

    /** Pointers comparison operator.
     *
     * @return @a true if both pointers are equal.
     */
    inline bool
    operator ==(const Ptr<T> &ptr) const
    {
        return _obj == ptr._obj;
    }

    /** Pointers comparison operator.
     *
     * @return @a true if pointers are not equal.
     */
    inline bool
    operator !=(const Ptr<T> &ptr) const
    {
        return _obj != ptr._obj;
    }

    /** Boolean casting operator. Can be used for null-pointer checks. */
    inline
    operator bool() const
    {
        return _obj != nullptr;
    }

    /** Dereference operator. Will throw @ref NullPtrError if the pointer is
     * currently storing null.
     *
     * @return Referenced object.
     */
    inline T &
    operator *() const
    {
        if (UNLIKELY(!_obj)) {
            throw NullPtrError();
        }
        return *_obj;
    }

    /** Member access operator. Will throw @ref NullPtrError if the pointer is
     * currently storing null.
     *
     * @return Pointer to referenced object.
     */
    inline T *
    operator ->() const
    {
        if (UNLIKELY(!_obj)) {
            throw NullPtrError();
        }
        return _obj;
    }
};

} /* namespace triton */

#endif /* PTR_H_ */
