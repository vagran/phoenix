/*
 * /phoenix/include/common/RBTree.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file RBTree.h
 * Red-Black trees interface.
 */

#ifndef RBTREE_H_
#define RBTREE_H_

/** Base class for red-black tree implementation. */
class RBTreeBase {
public:
    /** Tree node represented by this class. */
    class EntryBase {
    protected:
        friend class RBTreeBase;

        u8 isRed:1, /**< The node is red. */
           isWired:1; /**< The node is in a tree. */
        EntryBase *parent, *child[2];

        EntryBase() { isWired = false; }
    };

    /** Validate the tree. This method is intended for tree implementation
     * troubleshooting and normally is not required to be used.
     *
     * @return @a true if the tree is valid red-black tree, @a false if there
     *      are some rules violations.
     */
    bool Validate();

protected:

    RBTreeBase();

    /** This method should be overloaded in derived class. It must compare two
     * nodes.
     * @param e1 The first node to compare.
     * @param e2 The second node to compare.
     * @return Positive value if @a e1 greater than @a e2, negative value if
     *      @a e1 less than @a e2, zero if @a e1 equal to @a e2.
     */
    virtual int Compare(EntryBase *e1, EntryBase *e2) = 0;

    /** Insert node in the tree. This method either inserts the node or finds
     * existing node with the same key.
     *
     * @param node Node to insert.
     * @return Either @a node if it was inserted or existing node with the
     *      same key (@a node is not inserted in the tree in such case).
     */
    EntryBase *InsertNode(EntryBase *node);

    /** Get next tree node during the tree traversal.
     *
     * @param node Previously visited node. Can be NULL to get the first node.
     * @return Next node. NULL if all nodes traversed.
     */
    EntryBase *GetNextNode(EntryBase *node = 0);

private:
    /** Root node. */
    EntryBase *_root;
    /** Total number of nodes in the tree. */
    size_t _nodesCount;
    /** Tree generation, incremented after each change. */
    unsigned _generation;

    /** Re-balance the tree after insertion. This function can be called
     * recursively. It must be called only if there is RB balancing rules
     * violations. In particular only one violation must be present upon this
     * function calling - provided node is red and its parent is also red.
     *
     * @param node Inserted or lastly balanced node. Its parent should not be
     *      root and should be red.
     */
    void _RebalanceInsertion(EntryBase *node);

    /** Check if re-balancing after insertion is required for the provided
     * node. Call @ref _RebalanceInsertion method if required.
     *
     * @param node Node to check. Node should be red.
     */
    inline void _CheckRebalanceInsertion(EntryBase *node) {
        ASSERT(node->isRed);
        if (node->parent && node->parent->isRed && node->parent->parent) {
            _RebalanceInsertion(node);
        }
    }
};

/** Implementation template for red-black tree class.
 * Usage example:
 * @code
 * class MyItem {
 * public:
 *
 *    int Compare(MyItem &item);
 *    typedef class RBTree<Item, &MyItem::Compare> MyTree;
 * private:
 *    friend class RBTree<MyItem, &MyItem::Compare>;
 *    MyTree::Entry _rbEntry;
 * };
 *
 * MyItem::MyTree tree;
 * @endcode
 *
 * @param T Class for objects which are stored in a tree.
 * @param Comparator method of class @a T which can be used to compare two
 *      objects. It should have the following prototype:
 *      @code
 *      int Compare(T &obj);
 *      @endcode
 *      Its @a obj argument is another object against this object should be
 *      compared. The method must return positive value if this object is
 *      greater than the provided one, negative value if it is less, and zero
 *      if they are equal.
 * @param key_t Optional type for key. This parameter can be provided if look
 *      up by key is required.
 * @param KeyComparator Optional method to compare object with a key value.
 */
template <class T, int (T::*Comparator)(T &obj)>
          //typename key_t = int, int (T::*KeyComparator)(key_t &key) = 0>
class RBTree : public RBTreeBase {
public:
    class Entry : public EntryBase {
    protected:
        friend class RBTree;

        T *obj;
    };

    inline RBTree() : RBTreeBase() { }

    virtual int Compare(EntryBase *e1, EntryBase *e2)
    {
        return (static_cast<Entry *>(e1)->obj->*Comparator)(*static_cast<Entry *>(e2)->obj);
    }

    /** Try to insert an object in the tree. The object is inserted only if
     * there is no another object with the same key in the tree.
     *
     * @param obj Object to insert.
     * @param e Pointer to the tree entry.
     * @return Pointer to inserted object. It is either @a obj if the object
     *      was inserted or pointer to another existing object with the same
     *      key (@a obj is not inserted in such case).
     */
    inline T *InsertProbe(T *obj, Entry *e)
    {
        e->obj = obj;
        return static_cast<Entry *>(InsertNode(e))->obj;
    }

    /** Insert an object in the tree. The object is inserted only if
     * there is no another object with the same key in the tree.
     *
     * @param obj Object to insert.
     * @param e Pointer to the tree entry.
     * @return Pointer to inserted object. It is either @a obj if the object
     *      was inserted or pointer to another existing object with the same
     *      key (@a obj is not inserted in such case).
     */
    inline T *Insert(T *obj, Entry *e)
    {
        e->obj = obj;
        if (static_cast<Entry *>(InsertNode(e)) != e) {
            return 0;
        }
        return obj;
    }

    inline T *Lookup();

    /* Iteration interface. */

    class Iterator {
    public:
        inline Iterator(RBTree &tree, bool isStart) :
            _tree(tree)
        {
            if (isStart) {
                e = static_cast<Entry *>(_tree.GetNextNode());
            } else {
                e = 0;
            }
        }

        inline bool operator !=(Iterator &iter) { return e != iter.e; }

        inline void operator ++() {
            if (e) {
                e = static_cast<Entry *>(_tree.GetNextNode(e));
            }
        }

        inline T &operator *() { return *e->obj; }

    private:
        RBTree &_tree;
        Entry *e;
    };

    inline Iterator begin() { return Iterator(*this, true); }
    inline Iterator end() { return Iterator(*this, false); }
};

template <class T, int (T::*Comparator)(T &obj)>
static inline typename RBTree<T, Comparator>::Iterator
begin(RBTree<T, Comparator> &tree)
{
    return tree.begin();
}

template <class T, int (T::*Comparator)(T &obj)>
static inline typename RBTree<T, Comparator>::Iterator
end(RBTree<T, Comparator> &tree)
{
    return tree.end();
}

#endif /* RBTREE_H_ */
