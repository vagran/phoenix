/*
 * /phoenix/include/common/RBTree.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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
    /** Validate the tree. This method is intended for tree implementation
     * troubleshooting and normally is not required to be used.
     *
     * @return @a true if the tree is valid red-black tree, @a false if there
     *      are some rules violations or dis-integrity.
     */
    bool Validate();

protected:
    /** Tree node represented by this class. */
    class EntryBase {
    protected:
        friend class RBTreeBase;

        u8 isRed:1, /**< The node is red. */
           isWired:1; /**< The node is in a tree. */
        EntryBase *parent, *child[2];

        EntryBase() { isWired = false; }
    };

    RBTreeBase();

    /** This method should be overloaded in derived class. It must compare two
     * nodes.
     * @param e1 The first node to compare.
     * @param e2 The second node to compare.
     * @return Positive value if @a e1 greater than @a e2, negative value if
     *      @a e1 less than @a e2, zero if @a e1 equal to @a e2.
     */
    virtual int Compare(EntryBase *e1, EntryBase *e2) = 0;

    /** This method should be overloaded in derived class. It must compare a
     * node with a key.
     * @param e Node to compare.
     * @param key Pointer to a key.
     * @return Positive value if @a key is greater than @a e, negative value if
     *      @a key is less than @a e, zero if @a key is equal to @a e.
     */
    virtual int Compare(EntryBase *e, void *key) = 0;

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

    /** Lookup tree node by key.
     *
     * @param key Pointer to key.
     * @return Pointer to found node, @a 0 if nothing is found.
     */
    EntryBase *Lookup(void *key);

    /** Delete a node from the tree.
     *
     * @param entry Node to delete.
     */
    void Delete(EntryBase *entry);

    /** Get the node with the lowest value.
     *
     * @return Node with the lowest value, NULL if the tree is empty.
     */
    EntryBase *Lowest();

    /** Get the node with the highest value.
     *
     * @return Node with the highest value, NULL if the tree is empty.
     */
    EntryBase *Highest();

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

    /** Re-balance the tree after deletion and detach replacement entry.
     * @param node Replacement node which must be detached.
     */
    void _RebalanceDeletion(EntryBase *node);

    /** Rotate subtree around the specified node in specified direction.
     *
     * @param node Node to rotate around.
     * @param dir Rotation direction. If 0 then right child of the node will
     *      become its parent, left if 1.
     */
    inline void _Rotate(EntryBase *node, int dir) {
        ASSERT(dir == 0 || dir == 1);
        EntryBase *x = node->child[dir];
        ASSERT(x);
        x->parent = node->parent;
        if (node->parent) {
            if (node->parent->child[0] == node) {
                node->parent->child[0] = x;
            } else {
                ASSERT(node->parent->child[1] == node);
                node->parent->child[1] = x;
            }
        } else {
            ASSERT(_root == node);
            _root = x;
        }

        node->child[dir] = x->child[!dir];
        if (node->child[dir]) {
            node->child[dir]->parent = node;
        }

        x->child[!dir] = node;
        node->parent = x;
    }

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
 *    int Compare(int key);
 *    typedef class RBTree<Item, &MyItem::Compare, int, &MyItem::Compare> MyTree;
 * private:
 *    friend class RBTree<MyItem, &MyItem::Compare, int, &MyItem::Compare>;
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
 * @param key_t Type for key.
 * @param KeyComparator Method to compare object with a key value. The method
 *      must return positive value if key value is greater than this object
 *      value, negative value if it is less, and zero if they are equal. It
 *      should have the following prototype:
 *      @code
 *      int Compare(key_t &key);
 *      @endcode
 */
template <class T, int (T::*Comparator)(T &obj),
          typename key_t, int (T::*KeyComparator)(key_t &key)>
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

    virtual int Compare(EntryBase *e, void *key)
    {
        if (!KeyComparator) {
            FAULT("Key comparator not provided");
        }
        return (static_cast<Entry *>(e)->obj->*KeyComparator)(*static_cast<key_t *>(key));
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

    /** Lookup object by a key.
     *
     * @param key Key for lookup.
     * @return Pointer to found object, 0 if not found.
     */
    inline T *Lookup(key_t &key)
    {
        EntryBase *e = RBTreeBase::Lookup(&key);
        if (!e) {
            return 0;
        }
        return static_cast<Entry *>(e)->obj;
    }

    /** Delete a node by its entry in user object.
     *
     * @param e Tree entry of a node to delete.
     */
    inline void Delete(Entry *e)
    {
        RBTreeBase::Delete(static_cast<EntryBase *>(e));
    }

    /** Delete a node by its key.
     *
     * @param key Key of the node to delete.
     * @return Corresponding object if found, 0 if no such object in the tree.
     */
    inline T *Delete(key_t &key)
    {
        EntryBase *e = RBTreeBase::Lookup(&key);
        if (!e) {
            return 0;
        }
        RBTreeBase::Delete(e);
        return static_cast<Entry *>(e)->obj;
    }

    /** Get the object with the lowest value.
     *
     * @return Object with the lowest value, NULL if the tree is empty.
     */
    inline T *Lowest() {
        EntryBase *e = RBTreeBase::Lowest();
        if (!e) {
            return 0;
        }
        return static_cast<Entry *>(e)->obj;
    }

    /** Get the object with the highest value.
     *
     * @return Object with the highest value, NULL if the tree is empty.
     */
    inline T *Highest() {
        EntryBase *e = RBTreeBase::Highest();
        if (!e) {
            return 0;
        }
        return static_cast<Entry *>(e)->obj;
    }

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

template <class T, int (T::*Comparator)(T &obj),
          typename key_t, int (T::*KeyComparator)(key_t &key)>
static inline typename RBTree<T, Comparator, key_t, KeyComparator>::Iterator
begin(RBTree<T, Comparator, key_t, KeyComparator> &tree)
{
    return tree.begin();
}

template <class T, int (T::*Comparator)(T &obj),
          typename key_t, int (T::*KeyComparator)(key_t &key)>
static inline typename RBTree<T, Comparator, key_t, KeyComparator>::Iterator
end(RBTree<T, Comparator, key_t, KeyComparator> &tree)
{
    return tree.end();
}

#endif /* RBTREE_H_ */
