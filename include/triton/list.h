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

namespace triton_internal {

/** Base class for Triton lists. */
class ListBase {
private:
    /** Base class for list node. */
    class NodeBase {

    };

    template <typename T, class Enable = void>
    class Node: public NodeBase {
    public:
        T value;
    };

    size_t _numNodes;

protected:

    inline size_t
    __len__()
    {
        return _numNodes;
    }

public:


};

template <typename T>
class ListBase::Node<T *, enable_if<is_triton_obj<T>()>>:
public ListBase::NodeBase {
public:
     Ptr<T> value;
};

} /* namespace triton_internal */

template <typename T, class AllocatorT = Allocator<T>>
class List: public triton_internal::ListBase, public Sequence<T>, public Iterable {
private:

    typedef triton_internal::ListBase::Node<T> Node;

    typedef typename AllocatorT::template Rebind<Node> NodeAllocator;
public:
    /** Type of index of value in a list. */
    typedef typename Sequence<T>::index_t index_t;

    virtual const char *
    __name__() const
    {
        return "List";
    }

    virtual Object::hash_t
    __hash__() const
    {
        //XXX
        return 1;
    }

    virtual size_t
    __len__()
    {
        return ListBase::__len__();
    }

    virtual T &
    __min__()
    {
        //XXX
        throw ValueError();
    }

    virtual T &
    __max__()
    {
        //XXX
        throw ValueError();
    }

    virtual index_t
    index(T &&value)
    {
        //XXX
        return 0;
    }

    virtual size_t
    count(T &&value)
    {
        //XXX
        return 0;
    }
};

} /* namespace triton */

#endif /* LIST_H_ */
