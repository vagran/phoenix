/*
 * /phoenix/include/triton.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file triton.h
 * Main include file for Phoenix C/C++ standard library - Triton library. It
 * provides most common classes which should be used by all Phoenix code such
 * as strings, lists, maps etc.
 */

#ifndef TRITON_H_
#define TRITON_H_

#include <triton/std.h>
#include <triton/utils.h>
#include <triton/classes.h>
#include <triton/object.h>
#include <triton/allocator.h>

/** All Triton provided entities are defined in this namespace. */
namespace triton {

/* Triton built-in functions. */

/** Get hash value of an object.
 *
 * @param obj Triton object or a type from which it can be created.
 * @return Number of elements currently stored in container object.
 */
template <typename T>
constexpr inline Object::hash_t
hash(T &&obj);

/** Get number of elements in container object.
 *
 * @param obj Container object.
 * @return Number of elements currently stored in container object.
 */
template <typename T>
constexpr inline size_t
len(T &&obj);

/** Start iteration over iterable object items.
 *
 * @param obj Object to iterate on.
 * @return Iterator object.
 */
template <typename T>
constexpr inline auto
iter(T &&obj) -> decltype(object(forward<T>(obj)).__iter__());

/** Get next value from iterator object. @ref StopIteration exception is thrown
 * if no more items to iterate.
 *
 * @param it Iterator object.
 * @return Next item from the collection being iterated.
 */
template <typename IterT>
constexpr inline IterT &
next(Iterator<IterT> &it);

} /* namespace triton */

#include <triton/exception.h>
#include <triton/ptr.h>
#include <triton/iterator.h>
#include <triton/container.h>
#include <triton/sequence.h>
#include <triton/numeric.h>
#include <triton/tuple.h>
#include <triton/list.h>

namespace triton {

/* Triton built-in functions implementation should follow all Triton classes
 * declarations. The implementation depends on overloaded object() functions
 * provided by each triton class.
 */

template <typename T>
constexpr inline Object::hash_t
hash(T &&obj)
{
    return object(forward<T>(obj)).__hash__();
}

template <typename T>
constexpr inline size_t
len(T &&obj)
{
    return object(forward<T>(obj)).__len__();
}

template <typename T>
constexpr inline auto
iter(T &&obj) -> decltype(object(forward<T>(obj)).__iter__())
{
    return object(forward<T>(obj)).__iter__();
}

template <typename IterT>
constexpr inline IterT &
next(Iterator<IterT> &it)
{
    return it.__next__();
}

template <typename Tobj, class AllocatorT = Allocator<int>>
constexpr inline auto
list(Tobj &&obj) ->
List<typename decltype(object(forward<Tobj>(obj)).__iter__())::ValueType, AllocatorT>
{
    return List<typename decltype(object(forward<Tobj>(obj)).__iter__())::ValueType, AllocatorT>(object(forward<Tobj>(obj)));
}

} /* namespace triton */

#endif /* TRITON_H_ */
