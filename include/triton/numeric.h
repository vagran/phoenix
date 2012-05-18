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
        remove_cv_ref<enable_if<is_numeric<T>(), T>> value;
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

template <typename T>
class Numeric<T, enable_if<is_integral<T>()>>:
public triton_internal::NumericInt<remove_cv_ref<T>> {
private:
    typedef triton_internal::NumericInt<remove_cv_ref<T>> BaseType;
public:
    template <typename... Args>
    Numeric(Args &&... args) : BaseType(forward<Args>(args)...) {}
};

template <typename T>
class Numeric<T, enable_if<is_float<T>()>>:
public triton_internal::NumericFloat<remove_cv_ref<T>> {
private:
    typedef triton_internal::NumericFloat<remove_cv_ref<T>> BaseType;
public:
    template <typename... Args>
    Numeric(Args &&... args) : BaseType(forward<Args>(args)...) {}
};

/** Numeric class for characters. */
typedef Numeric<char> Char;
/** Numeric class for unsigned characters. */
typedef Numeric<unsigned char> UChar;
/** Numeric class for integers. */
typedef Numeric<int> Int;
/** Numeric class for unsigned integers. */
typedef Numeric<unsigned int> UInt;
/** Numeric class for long integers. */
typedef Numeric<long> Long;
/** Numeric class for unsigned long integers. */
typedef Numeric<unsigned long> ULong;
/** Numeric class for long long integers. */
typedef Numeric<long> LongLong;
/** Numeric class for unsigned long long integers. */
typedef Numeric<unsigned long> ULongLong;

/** Numeric class for floating point numbers. */
typedef Numeric<float> Float;
/** Numeric class for floating point numbers with double precision. */
typedef Numeric<double> Double;
/** Numeric class for floating point numbers with long double precision. */
typedef Numeric<long double> LongDouble;

template <typename T>
inline Numeric<T>
object(T &&value)
{
    return Numeric<T>(value);
}

} /* namespace triton */

#endif /* NUMERIC_H_ */
