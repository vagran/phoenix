/*
 * /phoenix/include/triton/classes.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file classes.h
 * Triton classes forward declarations and constraints.
 */

#ifndef CLASSES_H_
#define CLASSES_H_

namespace triton {

/* Forward declaration of all Triton classes. */

/** Base class for all Triton objects. All the library and client code objects
 * should be derived from this class.
 */
class Object;

/** Base class for all container classes. When Triton objects are stored in
 * containers, @ref Ptr class should be used for that.
 */
template <typename T>
class Container;

/** Base class for all sequence classes - containers which store items in a
 * sequenced manner - each items has permanent index while sequence is not
 * modified. Type of item is defined by @a T template parameter.
 */
template <typename T>
class Sequence;

/** Base class for all iterable classes - i.e. containers which support their
 * elements iteration.
 */
template <typename T>
class Iterable;

template <typename T>
class Iterator;

/** Triton wrapper class for all numeric types. */
template <typename T, class Enable = enable_if<is_numeric<T>()>>
class Numeric;

/** Tuple container stores arbitrary amount of different type values. */
template <class... components>
class Tuple;

/** List is a collection type for indexed storage of provided set of values.
 * Stored values type defined by @a T template parameter. Back-end for memory
 * allocations
 */
template <typename T, class AllocatorT>
class List;

namespace triton_internal {

template <typename T>
struct is_triton_obj_impl {
    static const bool value = false;
};

template <>
struct is_triton_obj_impl<Object> {
    static const bool value = true;
};

template <typename T>
struct is_triton_obj_impl<Container<T>> {
    static const bool value = true;
};

template <typename T>
struct is_triton_obj_impl<Iterable<T>> {
    static const bool value = true;
};

template <typename T>
struct is_triton_obj_impl<Iterator<T>> {
    static const bool value = true;
};

template <typename T>
struct is_triton_obj_impl<Sequence<T>> {
    static const bool value = true;
};

template <typename T>
struct is_triton_obj_impl<Numeric<T>> {
    static const bool value = true;
};

template <typename... Components>
struct is_triton_obj_impl<Tuple<Components...>> {
    static const bool value = true;
};

template <typename T, typename Allocator>
struct is_triton_obj_impl<List<T, Allocator>> {
    static const bool value = true;
};

} /* namespace triton_internal */

/** Check if provided type is Triton object class. */
template <typename T>
constexpr bool
is_triton_obj()
{
    return triton_internal::is_triton_obj_impl<remove_cv_ref<T>>::value;
}

} /* namespace triton */

#endif /* CLASSES_H_ */
