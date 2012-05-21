/*
 * /phoenix/include/triton/container.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file container.h
 * Triton container class definition.
 */

#ifndef CONTAINER_H_
#define CONTAINER_H_

namespace triton {

namespace triton_internal {

/** Helper for definition of actually stored type in container objects. */
template <typename T, typename Enable = void>
struct container_value_type_impl {
    typedef T ValueType;
};

/** Pointers to Triton objects should be represented by smart pointer. */
template <typename T>
struct container_value_type_impl<T *, enable_if<is_triton_obj<T>()>> {
    typedef Ptr<T> ValueType;
};

} /* namespace triton_internal */

template <typename T>
class Container: public Object {
public:
    /** Actually stored type. */
    typedef typename triton_internal::container_value_type_impl<T>::ValueType ValueType;

};

} /* namespace triton */

#endif /* CONTAINER_H_ */
