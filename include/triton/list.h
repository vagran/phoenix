/*
 * /phoenix/include/triton/list.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file list.h
 * Triton lists implementation.
 */

#ifndef LIST_H_
#define LIST_H_

namespace triton {

/** List is a collection type for indexed storage of provided set of values.
 * Stored values type defined by @a T template parameter. Back-end for memory
 * allocations
 */
template <typename T, class AllocatorT = Allocator<T>>
class List: public Iterable {
private:

    class Node {

    };

    typedef typename AllocatorT::template Rebind<Node> NodeAllocator;
public:
};

} /* namespace triton */

#endif /* LIST_H_ */
