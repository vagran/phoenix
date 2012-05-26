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

    /** Current number of nodes in the list. */
    long _numNodes = 0;
    /** First node in the list. */
    NodeBase *_firstNode = nullptr;

    inline size_t
    __len__() const
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

    /** List items iterator. */
    class ListIterator: public IteratorImpl<ValueType> {
    private:
        typedef List<T, AllocatorT> ListT;
        const ListT &_list;
        ListT::Node *_nextNode;
        index_t _idx, _endIdx;
    public:
        ListIterator(const ListT &list, index_t start = 0,
                     index_t end = Sequence<T>::MAX_INDEX) : _list(list)
        {
            if (start >= 0) {
                _idx = start;
            } else {
                _idx = list._numNodes + start;
            }
            if (end == Sequence<T>::MAX_INDEX) {
                _endIdx = list._numNodes;
            } else if (end >= 0) {
                _endIdx = end;
            } else {
                _endIdx = list._numNodes + end;
            }
            _nextNode = static_cast<Node *>(_list._firstNode);
            index_t curIdx = 0;
            while (_nextNode && curIdx < _idx) {
                if (_nextNode->next == _list._firstNode) {
                    _nextNode = nullptr;
                    break;
                }
                _nextNode = static_cast<Node *>(_nextNode->next);
                curIdx++;
                if (curIdx >= _endIdx) {
                    _nextNode = nullptr;
                    break;
                }
            }
            this->_hasNext = _nextNode ? true : false;
        }

        virtual
        ValueType &
        __next__()
        {
            if (_nextNode) {
                ValueType &v = _nextNode->value;
                _idx++;
                if (_nextNode->next == _list._firstNode ||
                    _idx >= _endIdx) {

                    _nextNode = nullptr;
                    this->_hasNext = false;
                } else {
                    _nextNode = static_cast<Node *>(_nextNode->next);
                }
                return v;
            }
            throw StopIteration();
        }
    };

    inline
    List() {}

    /** Create list from iterable object. */
    inline
    List(const Iterable<ValueType> &it)
    {
        for (auto &value: it) {
            append(value);
        }
    }

    /** Copy constructor. */
    inline
    List(const List<T, AllocatorT> &list):
    List(static_cast<const Iterable<ValueType> &>(list)) {}

    /** Move constructor. */
    inline
    List(List<T, AllocatorT> &&list)
    {
        _firstNode = list._firstNode;
        _numNodes = list._numNodes;
        list._firstNode = nullptr;
        list._numNodes = 0;
    }

    /** Construct list from initializer list with values.
     *
     * @param il
     */
    inline
    List(const InitList<T> &il)
    {
        for (auto &value: il) {
            append(value);
        }
    }

    inline
    ~List()
    {
        _ClearAll();
    }

    /** Assignment for initializer list. */
    List<T, AllocatorT> &
    operator =(const InitList<T> &il)
    {
        _ClearAll();
        for (auto &value: il) {
            append(value);
        }
        return *this;
    }

    /** Copy assignment. */
    List<T, AllocatorT> &
    operator =(const List<T, AllocatorT> &list)
    {
        _ClearAll();
        for (auto &value: list) {
            append(value);
        }
        return *this;
    }

    /** Move assignment. */
    List<T, AllocatorT> &
    operator =(List<T, AllocatorT> &&list)
    {
        _ClearAll();
        _firstNode = list._firstNode;
        _numNodes = list._numNodes;
        list._firstNode = nullptr;
        list._numNodes = 0;
        return *this;
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
    __len__() const
    {
        return ListBase::__len__();
    }

    virtual
    Iterator<ValueType>
    __iter__(bool endIterator = false) const
    {
        Iterator<ValueType> it;
        if (endIterator) {
            return it;
        }
        it.template Assign<ListIterator>(*this);
        return it;
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
    index(const T &value, index_t start = 0, index_t end = Sequence<T>::MAX_INDEX)
    {
        ListIterator it(*this, start, end);
        if (start < 0) {
            start = _numNodes + start;
        }
        while (it.__hasNext__()) {
            if (value == it.__next__()) {
                return start;
            }
            start++;
        }
        throw ValueError(/* XXX "x not in list" */);
    }

    virtual size_t
    count(const T &value)
    {
        size_t num = 0;
        for (auto &item: *this) {
            if (value == item) {
                num++;
            }
        }
        return num;
    }

    virtual void
    append(const T &value)
    {
        NodeBase *node = _alloc.Allocate(value);
        _Append(node);
    }

    virtual void
    extend(Iterable<T> &it)
    {
        //XXX not implemented
    }

    virtual void
    insert(index_t idx, const T &value)
    {
        NodeBase *node = _alloc.Allocate(value);
        _Insert(idx, node);
    }
};

template <typename T, typename AllocatorT>
constexpr inline List<T, AllocatorT> &
object(List<T, AllocatorT> &obj)
{
    return obj;
}

template <typename T, typename AllocatorT>
constexpr inline List<T, AllocatorT> &&
object(List<T, AllocatorT> &&obj)
{
    return static_cast<List<T, AllocatorT> &&>(obj);
}

template <typename T>
constexpr inline List<T>
object(const InitList<T> &il)
{
    return List<T>(il);
}

template <typename T, class AllocatorT = Allocator<T>>
constexpr inline List<T>
list(const InitList<T> &il)
{
    return List<T, AllocatorT>(il);
}

} /* namespace triton */

#endif /* LIST_H_ */
