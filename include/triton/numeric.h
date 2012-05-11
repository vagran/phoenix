/*
 * /phoenix/include/triton/numeric.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file numeric.h
 * Triton classes for numeric types.
 */

#ifndef NUMERIC_H_
#define NUMERIC_H_

namespace triton {

namespace triton_internal {

/** Class which represents generic numeric value, both integer and floating
 * point.
 */
template <typename T>
class Numeric: public Object {
protected:
    union Value {
        T value;
        hash_t hash;
    } _v;
public:

    inline
    Numeric(T value)
    {
        _v.hash = 0;
        _v.value = value;
    }

    template <typename Targ>
    inline
    Numeric(const Numeric<Targ> &value)
    {
        _v.hash = 0;
        _v.value = value._v.value;
    }

    virtual hash_t
    __hash__() const
    {
        return _v.hash;
    }

    inline
    operator T()
    {
        return _v.value;
    }

    template <typename Targ>
    inline Numeric<T> &
    operator =(const Numeric<Targ> &value)
    {
        _v.hash = 0;
        _v.value = value._v.value;
    }
};

template <typename T>
class NumericInt: public Numeric<T> {
public:

    inline NumericInt(T value) : Numeric<T>(value) {}

    template <typename Targ>
    inline NumericInt<T> &
    operator &=(Targ value)
    {
        this->_v.value &= value;
        return *this;
    }

    template <typename Targ>
    inline NumericInt<T> &
    operator |=(Targ value)
    {
        this->_v.value |= value;
        return *this;
    }

    template <typename Targ>
    inline NumericInt<T> &
    operator ^=(Targ value)
    {
        this->_v.value ^= value;
        return *this;
    }
};

} /* namespace triton_internal */

/** Numeric class for integers. */
typedef triton_internal::NumericInt<int> Int;
/** Numeric class for unsigned integers. */
typedef triton_internal::NumericInt<unsigned int> UInt;
/** Numeric class for long integers. */
typedef triton_internal::NumericInt<long> Long;
/** Numeric class for unsigned long integers. */
typedef triton_internal::NumericInt<unsigned long> ULong;

inline Object::hash_t
hash(int value)
{
    return Int(value).__hash__();
}

inline Object::hash_t
hash(unsigned int value)
{
    return UInt(value).__hash__();
}

inline Object::hash_t
hash(long value)
{
    return Long(value).__hash__();
}

inline Object::hash_t
hash(unsigned long value)
{
    return ULong(value).__hash__();
}

} /* namespace triton */

#endif /* NUMERIC_H_ */
