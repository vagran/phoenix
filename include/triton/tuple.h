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

/** Helper class storing tuple components. */
template <typename... components>
class TupleStorage;

template <> class TupleStorage<> { };

template <typename T, typename... components>
class TupleStorage<T, components...>: public TupleStorage<components...> {
public:
    /** This component of a tuple. */
    T value;
    /** Type of base class for this one. */
    typedef TupleStorage<components...> BaseType;

    /** Construct tuple storage from provided values. */
    inline
    TupleStorage(add_const_reference<T> firstValue,
                 add_const_reference<components>... restValues) :
        BaseType(restValues...), value(firstValue) {}

    TupleStorage(const TupleStorage<T, components...> &src) :
        BaseType(static_cast<const BaseType &>(src)), value(src.value) {}
};

/** Helper class for accessing values of a tuple. */
template <int idx, typename T, typename... components>
class TupleGetter {
public:
    static inline TupleType<idx, T, components...> &
    Get(TupleStorage<T, components...> &stg)
    {
        return TupleGetter<idx - 1, components...>::Get(stg);
    }

    static inline Object::hash_t
    __hash__(const TupleStorage<T, components...> &stg)
    {
        const Object::hash_t h = TupleGetter<idx - 1, components...>::__hash__(stg);
        //XXX use hash mix
        return hash(stg.value) ^ h;
    }
};

template <typename T, typename... components>
class TupleGetter<0, T, components...> {
public:
    static inline T &
    Get(TupleStorage<T, components...> &stg)
    {
        return stg.value;
    }

    static inline Object::hash_t
    __hash__(const TupleStorage<T, components...> &stg)
    {
        return hash(stg.value);
    }
};

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

    inline
    Tuple(const Tuple<components...> &t) : _values(t._values) {}

    virtual const char *
    __name__() const
    {
        return "Tuple";
    }

    /** Get tuple length.
     *
     * @return Number of values in the tuple.
     */
    virtual size_t
    __len__()
    {
        return sizeof...(components);
    }

    /** Get value from tuple.
     * @param idx Index of value to retrieve.
     * @return Reference to a value at specified index.
     * @code
     * t.get<2>() = 10;
     * @endcode
     */
    template <int idx>
    inline triton_internal::TupleType<idx, components...> &
    get()
    {
        return triton_internal::TupleGetter<idx, components...>::Get(_values);
    }

    /** Get hash value for a tuple. */
    virtual Object::hash_t
    __hash__() const
    {
        return triton_internal::
               TupleGetter<sizeof...(components) - 1, components...>::
               __hash__(_values);
    }
};

} /* namespace triton */

#endif /* TUPLE_H_ */
