/*
 * /phoenix/include/triton/utils.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file utils.h
 * Various helper utilities for Triton.
 */

#ifndef UTILS_H_
#define UTILS_H_

/** All Triton provided entities are defined in this namespace. */
namespace triton {

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
 * @code
 * template <class T>
 * typename enable_if<is_numeric<T>(), T>
 * foo(T t)
 * {
 *      return t;
 * }
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
