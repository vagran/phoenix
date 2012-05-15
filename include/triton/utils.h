/*
 * /phoenix/include/triton/utils.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file
 * Various helper utilities for Triton.
 */

#ifndef UTILS_H_
#define UTILS_H_

namespace triton {

/* ************************************************************************** */

namespace triton_internal {

template <typename T>
struct remove_const_impl {
    typedef T type;
};

template <typename T>
struct remove_const_impl<const T> {
    typedef T type;
};

template <typename T>
struct remove_volatile_impl {
    typedef T type;
};

template <typename T>
struct remove_volatile_impl<volatile T> {
    typedef T type;
};

template <typename T>
struct remove_ptr_impl {
    typedef T type;
};

template <typename T>
struct remove_ptr_impl<T *> {
    typedef T type;
};

template <typename T>
struct remove_ref_impl {
    typedef T type;
};

template <typename T>
struct remove_ref_impl <T &> {
    typedef T type;
};

template <typename T>
struct remove_ref_impl <T &&> {
    typedef T type;
};

} /* namespace triton_internal */

/** Remove @a const qualifier from provided type. */
template <typename T>
using remove_const = typename triton_internal::remove_const_impl<T>::type;

/** Remove @a volatile qualifier from provided type. */
template <typename T>
using remove_volatile = typename triton_internal::remove_volatile_impl<T>::type;

/** Remove both @a const and @a volatile qualifiers from provided type. */
template <typename T>
using remove_cv = typename triton_internal::remove_volatile_impl<
                  typename triton_internal::remove_const_impl<T>::type>::type;

/** Convert pointer type to the type pointed to. */
template <typename T>
using remove_ptr = typename triton_internal::remove_ptr_impl<T>::type;

/** Convert reference type (either lvalue or rvalue) to the referenced type. */
template <typename T>
using remove_ref = typename triton_internal::remove_ref_impl<T>::type;

/* ************************************************************************** */

namespace triton_internal {

template <typename T>
constexpr bool
is_integral_impl()
{
    return false;
}

#define TRITON_IS_INTEGRAL_IMPL(__type) \
template <> \
constexpr bool \
is_integral_impl<__type>() \
{ \
    return true; \
}

TRITON_IS_INTEGRAL_IMPL(char)
TRITON_IS_INTEGRAL_IMPL(unsigned char)
TRITON_IS_INTEGRAL_IMPL(wchar_t)
TRITON_IS_INTEGRAL_IMPL(int)
TRITON_IS_INTEGRAL_IMPL(unsigned)
TRITON_IS_INTEGRAL_IMPL(long)
TRITON_IS_INTEGRAL_IMPL(unsigned long)
TRITON_IS_INTEGRAL_IMPL(long long)
TRITON_IS_INTEGRAL_IMPL(unsigned long long)

template <typename T>
constexpr bool
is_float_impl()
{
    return false;
}

#define TRITON_IS_FLOAT_IMPL(__type) \
template <> \
constexpr bool \
is_float_impl<__type>() \
{ \
    return true; \
}

TRITON_IS_FLOAT_IMPL(float)
TRITON_IS_FLOAT_IMPL(double)
TRITON_IS_FLOAT_IMPL(long double)

template <typename T>
struct is_lvalue_ref_impl {
    static const bool value = false;
};

template <typename T>
struct is_lvalue_ref_impl<T &> {
    static const bool value = true;
};

template <typename T>
struct is_rvalue_ref_impl {
    static const bool value = false;
};

template <typename T>
struct is_rvalue_ref_impl<T &&> {
    static const bool value = true;
};

} /* namespace triton_internal */

/** Check if specified type is lvalue reference. */
template <typename T>
constexpr bool
is_lvalue_ref()
{
    return triton_internal::is_lvalue_ref_impl<T>::value;
}

/** Check if specified type is rvalue reference. */
template <typename T>
constexpr bool
is_rvalue_ref()
{
    return triton_internal::is_rvalue_ref_impl<T>::value;
}

/** Check if provided type is integral type. */
template <typename T>
constexpr bool
is_integral()
{
    return triton_internal::is_integral_impl<remove_cv<remove_ref<T>>>();
}

/** Check if provided type is floating point type. */
template <typename T>
constexpr bool
is_float()
{
    return triton_internal::is_float_impl<remove_cv<remove_ref<T>>>();
}

/** Check if provided type is numeric type (either integral or floating point). */
template <typename T>
constexpr bool
is_numeric()
{
    return triton_internal::is_integral_impl<remove_cv<remove_ref<T>>>() ||
           triton_internal::is_float_impl<remove_cv<remove_ref<T>>>();
}

/* ************************************************************************** */

/** Helper for arguments forwarding. Forwards lvalue @a arg.
 * @param arg lvalue to forward.
 * @return The parameter cast to the specified type.
 */
template <typename T>
constexpr T &&
forward(remove_ref<T> &arg)
{
    return static_cast<T &&>(arg);
}

/** Helper for arguments forwarding. Forwards rvalue @a arg.
 * @param arg rvalue to forward.
 * @return The parameter cast to the specified type.
 */
template <typename T>
constexpr T &&
forward(remove_ref<T> &&arg)
{
    static_assert(!is_lvalue_ref<T>(), "lvalue substituted");
    return static_cast<T &&>(arg);
}

/** Convert a value to an rvalue.
 *  @param arg A thing of arbitrary type.
 *  @return The parameter cast to an rvalue-reference to allow moving it.
*/
template <typename T>
constexpr remove_ref<T> &&
move(T &&arg)
{
    return static_cast<remove_ref<T> &&>(arg);
}

/* ************************************************************************** */

namespace triton_internal {

/* Conditional template instantiation helpers. Idea borrowed from Boost library. */

template <bool cond, class T = void>
struct enable_if_impl {
    typedef T type;
};

template <class T>
struct enable_if_impl<false, T> {};

template <bool cond, class T = void>
struct disable_if_impl {
    typedef T type;
};

template <class T>
struct disable_if_impl<true, T> {};

} /* namespace triton_internal */

/** Helper for template conditional instantiations.
 * @param cond Constant boolean value which indicates whether the template can
 *      be evaluated or not.
 * @param T Type which is returned by the template if condition is @a true.
 *      Otherwise it is evaluated as invalid expression which causes compiler
 *      to select another instantiation.
 * @return @a T argument if @a Cond is @a true.
 * Usage example:
 * @code
 * template <class T>
 * enable_if<is_numeric<T>(), T>
 * foo(T t)
 * {
 *      return t;
 * }
 * @endcode
 */
template <bool cond, class T = void>
using enable_if = typename triton_internal::enable_if_impl<cond, T>::type;

/** Helper for template conditional instantiations. Negated version if @ref
 * enable_if_c.
 */
template <bool cond, class T = void>
using disable_if = typename triton_internal::enable_if_impl<cond, T>::type;

/* ************************************************************************** */

/** Transform type to a constant. */
template <typename T>
using add_const = const T;

/** Transform type to a reference. */
template <typename T>
using add_reference = T &;

/** Transform type to a constant reference. */
template <typename T>
using add_const_reference = const T &;

/* ************************************************************************** */
/* Integral constant expressions manipulations. */

namespace triton_internal {

template <bool... values>
struct ice_or_impl;

template <>
struct ice_or_impl<>
{
    static const bool value = false;
};

template <bool... values>
struct ice_or_impl<true, values...>
{
    static const bool value = true;
};

template <bool... values>
struct ice_or_impl<false, values...>
{
    static const bool value = ice_or_impl<values...>::value;
};

template <bool... values>
struct ice_and_impl;

template <>
struct ice_and_impl<>
{
    static const bool value = true;
};

template <bool... values>
struct ice_and_impl<false, values...>
{
    static const bool value = false;
};

template <bool... values>
struct ice_and_impl<true, values...>
{
    static const bool value = ice_and_impl<values...>::value;
};

} /* namespace triton_internal */

/** Helper for boolean "or" operation with constant values.
 * Usage example:
 * @code
 * ...
 * ice_or<is_integer<T>, is_float<T>>()
 * ...
 * @endcode
 */
template <bool... values>
constexpr bool
ice_or()
{
    return triton_internal::ice_or_impl<values...>::value;
}

/** Helper for boolean "and" operation with constant values.
 * Usage example:
 * @code
 * ...
 * ice_and<is_integer<T>, is_float<T>>()
 * ...
 * @endcode
 */
template <bool... values>
constexpr bool
ice_and()
{
    return triton_internal::ice_and_impl<values...>::value;
}

/** Helper for boolean "not" operation with constant value.
 * Usage example:
 * @code
 * ...
 * ice_not<is_integer<T>>()
 * ...
 * @endcode
 */
template <bool value>
constexpr bool
ice_not()
{
    return true;
}

template <>
constexpr bool
ice_not<true>()
{
    return false;
}

/* ************************************************************************** */

} /* namespace triton */

#endif /* UTILS_H_ */
