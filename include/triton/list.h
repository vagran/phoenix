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
protected:
    /** Base class for list node. */
    class NodeBase {
    public:
        /* Nodes arranged in doubly-linked circular list. */
        NodeBase *next = this, *prev = this;

        inline
        NodeBase() {}

        void
        Link(NodeBase *prevNode);

        void
        Unlink();
    };

    template <typename T, class Enable = void>
    class Node: public NodeBase {
    public:
        T value;

        /** Construct node. All arguments are forwarded to encapsulated value
         * constructor.
         * @param args Arguments for encapsulated value constructor.
         */
        template <typename... Args>
        inline
        Node(Args&&... args) : NodeBase(), value(forward<Args>(args)...) {}
    };

private:
    /** Current number of nodes in the list. */
    long _numNodes = 0;
    /** First node in the list. */
    NodeBase *_firstNode = nullptr;

protected:

    inline size_t
    __len__()
    {
        return _numNodes;
    }

    /** Should be overloaded in derived class in order to properly deallocate
     * list nodes.
     *
     * @param node Node to free.
     */
    virtual void _FreeNode(NodeBase *node) = 0;

    /** Append specified node to the end of nodes list. */
    void _Append(NodeBase *node);
    /** Insert node at specified position. */
    void _Insert(long idx, NodeBase *node);
    /** Get node at specified position.
     * @param idx Index of node to retrieve. Negative values are counted from
     *      the list end. @ref IndexError is thrown if index is out of range.
     * @return Node at specified position.
     */
    NodeBase *_GetNode(long idx);
    /** Free all nodes. */
    void _ClearAll();
public:
    inline
    ListBase() {}

    ~ListBase() {}
};

/** Specialization triton objects pointers. They are properly stored using
 * reference counting.
 */
template <typename T>
class ListBase::Node<T *, enable_if<is_triton_obj<T>()>>:
public ListBase::NodeBase {
public:
     Ptr<T> value;

     inline
     Node(T *ptr) : NodeBase(), value(ptr) {}
};

} /* namespace triton_internal */

template <typename T, class AllocatorT = Allocator<T>>
class List:
    public triton_internal::ListBase, public Sequence<T>,
    public Iterable<typename Container<T>::ValueType> {
private:

    typedef triton_internal::ListBase::Node<T> Node;

    typedef typename AllocatorT::template Rebind<Node> NodeAllocator;

    NodeAllocator _alloc;

    virtual void
    _FreeNode(NodeBase *node)
    {
        _alloc.Free(static_cast<Node *>(node));
    }

public:
    /** Type of index of value in a list. */
    typedef typename Sequence<T>::index_t index_t;
    /** Type of stored value (can be different from template parameter because
     * of node template specialization.
     */
    typedef decltype(Node::value) ValueType;

    class ListIterator: public IteratorImpl<ValueType> {
    private:
        List<T, AllocatorT> &_list;
    public:
        ListIterator(List<T, AllocatorT> &list) : _list(list)
        {

        }

        virtual
        ValueType &
        __next__()
        {
            throw StopIteration();
        }
    };

    ~List()
    {
        _ClearAll();
    }

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

    virtual
    Iterator<ValueType>
    __iter__()
    {
        return Iterator<ValueType>().template Assign<ListIterator>(*this);
    }

    virtual ValueType &
    __min__()
    {
        //XXX
        throw ValueError();
    }

    virtual ValueType &
    __max__()
    {
        //XXX
        throw ValueError();
    }

    virtual ValueType &
    operator [](index_t idx)
    {
        return static_cast<Node *>(_GetNode(idx))->value;
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

    virtual void
    append(T &&value)
    {
        NodeBase *node = _alloc.Allocate(forward<T>(value));
        _Append(node);
    }

    virtual void
    extend(Iterable<T> &it) {}

    virtual void
    insert(index_t idx, T &&value)
    {
        NodeBase *node = _alloc.Allocate(forward<T>(value));
        _Insert(idx, node);
    }
};

} /* namespace triton */

#endif /* LIST_H_ */
