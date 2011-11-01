/*
 * /phoenix/include/std.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file std.h
 * This file contains required definitions from @ref std namespace.
 * The Phoenix does not use C++ standard library provided types however some
 * of @ref std namespace definitions are required in order to use some
 * language features (e.g. initializer lists).
 */

#ifndef STD_H_
#define STD_H_

/** Contains definitions required for some language features. */
namespace std {

/** Class required in order to support initializer lists. */
template<class _E>
class initializer_list {
public:
    typedef _E value_type;
    typedef const _E& reference;
    typedef const _E& const_reference;
    typedef size_t size_type;
    typedef const _E* iterator;
    typedef const _E* const_iterator;

private:
    iterator _M_array;
    size_type _M_len;

    /* The compiler can call a private constructor. */
    constexpr initializer_list(const_iterator __a, size_type __l) :
        _M_array(__a), _M_len(__l)
    {
    }

public:
    constexpr initializer_list() :
        _M_array(0), _M_len(0)
    {
    }

    /* Number of elements. */
    constexpr
    size_type size()
    {
        return _M_len;
    }

    /* First element. */
    constexpr
    const_iterator begin()
    {
        return _M_array;
    }

    /* One past the last element. */
    constexpr
    const_iterator end()
    {
        return begin() + size();
    }
};

/** Return an iterator pointing to the first element of the @ref initializer_list.
 *  @param __ils Initializer list.
 */
template<class _Tp>
constexpr const _Tp*
begin(initializer_list<_Tp> __ils)
{
    return __ils.begin();
}

/** Return an iterator pointing to one past the last element of the
 *      @ref initializer_list.
 *  @param __ils Initializer list.
 */
template<class _Tp>
constexpr const _Tp*
end(initializer_list<_Tp> __ils)
{
    return __ils.end();
}

} /* namespace std */

#endif /* STD_H_ */
