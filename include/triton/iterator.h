/*
 * /phoenix/include/triton/iterator.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file iterator.h
 * Triton iterators interfaces description.
 */

#ifndef ITERATOR_H_
#define ITERATOR_H_

namespace triton {

/** Interface for objects which support iteration over their contained elements. */
template <typename T>
class Iterable {
    //XXX
};

/** Objects of this class are created by iterable classes. The object controls
 * iteration over elements stored in iterable object.
 */
template <typename T>
class Iterator: public Object, public Iterable<T> {
public:
    inline bool operator !=(Iterator<T> &it) { return false; }
    inline void operator ++() {  }
    inline T &operator *() { return T(); }
};

} /* namespace triton */

#endif /* ITERATOR_H_ */
