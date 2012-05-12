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
        enable_if<is_numeric<T>(), T> value;
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
class NumericInt: public Numeric<enable_if<is_integral<T>(), T>> {
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

template <typename T>
class NumericFloat: public Numeric<enable_if<is_float<T>(), T>> {
public:

    inline NumericFloat(T value) : Numeric<T>(value) {}

};

} /* namespace triton_internal */

/** Numeric class for characters. */
typedef triton_internal::NumericInt<char> Char;
/** Numeric class for unsigned characters. */
typedef triton_internal::NumericInt<unsigned char> UChar;
/** Numeric class for integers. */
typedef triton_internal::NumericInt<int> Int;
/** Numeric class for unsigned integers. */
typedef triton_internal::NumericInt<unsigned int> UInt;
/** Numeric class for long integers. */
typedef triton_internal::NumericInt<long> Long;
/** Numeric class for unsigned long integers. */
typedef triton_internal::NumericInt<unsigned long> ULong;
/** Numeric class for long long integers. */
typedef triton_internal::NumericInt<long> LongLong;
/** Numeric class for unsigned long long integers. */
typedef triton_internal::NumericInt<unsigned long> ULongLong;

/** Numeric class for floating point numbers. */
typedef triton_internal::NumericFloat<float> Float;
/** Numeric class for floating point numbers with double precision. */
typedef triton_internal::NumericFloat<double> Double;
/** Numeric class for floating point numbers with long double precision. */
typedef triton_internal::NumericFloat<long double> LongDouble;

template <typename T>
inline Object::hash_t
hash(T value, enable_if<is_integral<T>(), T> dummy = 0)
{
    return triton_internal::NumericInt<T>(value).__hash__();
}

template <typename T>
inline Object::hash_t
hash(T value, enable_if<is_float<T>(), T> dummy = 0)
{
    return triton_internal::NumericFloat<T>(value).__hash__();
}

} /* namespace triton */

#endif /* NUMERIC_H_ */
