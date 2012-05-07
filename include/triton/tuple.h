/*
 * /phoenix/include/triton/tuple.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file tuple.h
 * Triton tuple type definition.
 */

#ifndef TUPLE_H_
#define TUPLE_H_

namespace triton {

/** Namespace for internal Triton helpers which should not be publicly used. */
namespace triton_internal {

/** Helper for determining number of values in a tuple.
 * @code
 * size = TupleSize<int, float, char>::size;
 * @endcode
 */
template <typename... components> struct TupleSize;

template <>
struct TupleSize<> {
    static const size_t size = 0;
};

template <typename T, typename... components>
struct TupleSize<T, components...> {
    static const size_t size = 1 + TupleSize<components...>::size;
};

/** Helper class storing tuple components. */
template <typename... components> class TupleStorage;

template <> class TupleStorage<> { };

template <typename T, typename... components>
class TupleStorage<T, components...>: private TupleStorage<components...> {
public:
    /** This component of a tuple. */
    T value;
    /** Type of base class for this one. */
    typedef TupleStorage<components...> BaseType;

    inline
    TupleStorage(add_const_reference<T> firstValue,
                 add_const_reference<components>... restValues) :
        BaseType(restValues...), value(firstValue) {}
};

/** Helper for retrieving type of tuple components at specified position.
 * @code
 * TupleTypeImpl<2, int, flow, char>::Type c = 'a';
 * @endcode
 */
template <int idx, typename T, typename... components>
struct TupleTypeImpl {
    typedef typename TupleTypeImpl<idx - 1, components...>::Type Type;
};

template <typename T, typename... components>
struct TupleTypeImpl<0, T, components...> {
    typedef T Type;
};

/** Helper for retrieving type of tuple components at specified position.
 * @code
 * TupleType<2, int, flow, char> c = 'a';
 * @endcode
 */
template <int idx, typename... components>
using TupleType = typename TupleTypeImpl<idx, components...>::Type;

}; /* namespace triton_internal */

/** Tuple container stores arbitrary amount of different type values. */
template <class... components>
class Tuple: public Container {
private:
    triton_internal::TupleStorage<components...> _values;
public:
    /** Get type of tuple component.
     *
     * @param idx Index of tuple component which type should be returned.
     * @code
     * decltype(t)::Type<2> v = 0;
     * @endcode
     */
    template <int idx>
    using Type = triton_internal::TupleType<idx, components...>;

    inline
    Tuple(components... values) : _values(values...) {}

    /** Get tuple length.
     *
     * @return Number of values in the tuple.
     */
    virtual size_t
    __len__()
    {
        return triton_internal::TupleSize<components...>::size;
    }
};

} /* namespace triton */

#endif /* TUPLE_H_ */
