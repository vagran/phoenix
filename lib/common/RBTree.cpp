/*
 * /phoenix/lib/common/RBTree.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file RBTree.cpp
 * Red-Black trees implementation.
 */

#include <sys.h>

RBTreeBase::RBTreeBase()
{
    _root = 0;
    _nodesCount = 0;
    _generation = 0;
}

RBTreeBase::EntryBase *
RBTreeBase::InsertNode(EntryBase *node)
{
    node->child[0] = 0;
    node->child[1] = 0;

    /* Special case - empty tree, insert root. */
    if (UNLIKELY(!_root)) {
        _root = node;
        node->parent = 0;
        node->isRed = false;
        ASSERT(!_nodesCount);
        _nodesCount++;
        _generation++;
        return node;
    }

    /* Firstly search for insertion point and insert the node. */
    EntryBase *parent = _root;
    while (true) {
        int cmp = Compare(node, parent);
        if (!cmp) {
            return parent;
        }
        if (parent->child[cmp > 0]) {
            parent = parent->child[cmp > 0];
        } else {
            parent->child[cmp > 0] = node;
            node->parent = parent;
            node->isRed = true;
            _nodesCount++;
            _generation++;
            break;
        }
    }

    /* Re-balance the tree if necessary. */
    if (node->parent->isRed) {
        _RebalanceInsertion(node);
    }
    /* Set root black if it was re-colored during re-balancing. */
    _root->isRed = false;
    return node;
}

void
RBTreeBase::_RebalanceInsertion(EntryBase *node)
{
    /* Validate entrance conditions. */
    ASSERT(node->isRed);
    ASSERT(node->parent);
    ASSERT(node->parent->isRed);
    ASSERT(node->parent->parent);

    /* We have two symmetric scenarios - new node is either in left or right
     * subtree of its grandparent.
     */
    ASSERT(node->parent->child[0] == node || node->parent->child[1] == node);
    ASSERT(node->parent->parent->child[0] == node->parent ||
           node->parent->parent->child[1] == node->parent);
    int dir = node->parent->parent->child[1] == node->parent;

    /* Node uncle. */
    EntryBase *y = node->parent->parent->child[!dir];
    if (y && y->isRed) {
        /* Case 1: uncle is red - just re-color nodes. */
        y->isRed = false;
        node->parent->isRed = false;
        node->parent->parent->isRed = true;
        /* Grandparent became red so re-balancing could be required again. */
        _CheckRebalanceInsertion(node->parent->parent);
    } else {
        int nodeDir = node->parent->child[1] == node;
        if (nodeDir == dir) {
            y = node->parent;
        } else {
            /* Case 3 - rotate parent and transform to case 2. */
            EntryBase *x = node->parent;
            y = node;
            x->child[!dir] = y->child[dir];
            if (x->child[!dir]) {
                x->child[!dir]->parent = x;
            }

            y->parent = x->parent;
            y->parent->child[dir] = y;

            y->child[dir] = x;
            x->parent = y;
        }
        /* Case 2 - perform grandparent rotation. */
        EntryBase *x = y->parent;

        x->isRed = true;
        y->isRed = false;

        x->child[dir] = y->child[!dir];
        if (x->child[dir]) {
            x->child[dir]->parent = x;
        }

        y->parent = x->parent;
        x->parent = y;
        y->child[!dir] = x;
        if (y->parent) {
            if (y->parent->child[0] == x) {
                y->parent->child[0] = y;
            } else {
                y->parent->child[1] = y;
            }
        } else {
            _root = y;
        }
    }
}

RBTreeBase::EntryBase *
RBTreeBase::GetNextNode(EntryBase *node)
{
    if (!node) {
        return _root;
    }
    if (node->child[0]) {
        return node->child[0];
    }
    if (node->child[1]) {
        return node->child[1];
    }

    while (node->parent) {
        if (node->parent->child[0] == node && node->parent->child[1]) {
            return node->parent->child[1];
        }
        node = node->parent;
    }
    return 0;
}
