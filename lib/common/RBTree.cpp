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
    ASSERT(!node->isWired);

    node->child[0] = 0;
    node->child[1] = 0;

    /* Special case - empty tree, insert root. */
    if (UNLIKELY(!_root)) {
        _root = node;
        node->parent = 0;
        node->isRed = false;
        node->isWired = true;
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
            node->isWired = true;
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
            _Rotate(node->parent, !dir);
        }
        /* Case 2 - perform grandparent rotation. */
        EntryBase *x = y->parent;

        x->isRed = true;
        y->isRed = false;

        _Rotate(x, dir);
    }
}

void
RBTreeBase::_RebalanceDeletion(EntryBase *node)
{
    EntryBase *replNode = node, *tmpNode;

    if (!node->parent) {
        ASSERT(node == _root);
        _root = 0;
        return;
    }

    do {
        /* Current is red leaf. */
        if (node->isRed && !node->child[0] && !node->child[1]) {
            /* Done */
            break;
        }

        int nodeDir;
        if (node->parent->child[0] == node) {
            nodeDir = 0;
        } else {
            ASSERT(node->parent->child[1] == node);
            nodeDir = 1;
        }

        /* Current is black with one red child. */
        if (!node->isRed &&
            (((tmpNode = node->child[0]) && tmpNode->isRed && !node->child[1]) ||
             ((tmpNode = node->child[1]) && tmpNode->isRed && !node->child[0]))) {

            tmpNode->parent = node->parent;
            if (nodeDir != -1) {
                node->parent->child[nodeDir] = tmpNode;
            }
            break;
        }

        do {
            /* Current sibling is red. */
            EntryBase *siblNode = node->parent->child[!nodeDir];
            if (siblNode->isRed) {
                /* Exchange colors of parent and sibling nodes. */
                node->parent->isRed = true;
                siblNode->isRed = false;

                /* Rotate around the parent. */
                _Rotate(node->parent, !nodeDir);
                continue;
            }

            /* Current sibling is black with two black children */
            if ((!siblNode->child[0] || !siblNode->child[0]->isRed) &&
                (!siblNode->child[1] || !siblNode->child[1]->isRed)) {

                /* Make sibling red. */
                siblNode->isRed = true;
                /* Make parent new current node. */
                node = node->parent;
                if (!node->isRed && node->parent) {
                    if (node->parent->child[0] == node) {
                        nodeDir = 0;
                    } else {
                        ASSERT(node->parent->child[1] == node);
                        nodeDir = 1;
                    }
                    continue;
                }
                /* Current is red - make it black and we are done. */
                node->isRed = false;
                break;
            }

            /* Current sibling is black with one or two red children. */
            EntryBase *farNephewNode = siblNode->child[!nodeDir];

            if (!farNephewNode->isRed) {
                /* Far nephew is black, rotate around the sibling. */
                _Rotate(siblNode, nodeDir);
                siblNode = node->parent->child[!nodeDir];
                farNephewNode = siblNode->child[!nodeDir];
            }

            /* Color the far nephew black, make the sibling color the same as
             * the color of its parent, color the parent black.
             */
            farNephewNode->isRed = false;
            siblNode->isRed = node->parent->isRed;
            node->parent->isRed = false;

            /* Rotate around the parent. */
            _Rotate(node->parent, !nodeDir);
            break;
        } while (node);
    } while(false);

    /* Detach replacement node. */
    if (replNode->parent->child[0] == replNode) {
        replNode->parent->child[0] = 0;
    } else {
        ASSERT(replNode->parent->child[1] == replNode);
        replNode->parent->child[1] = 0;
    }
}

void
RBTreeBase::Delete(EntryBase *node)
{
    ASSERT(node->isWired);
    /* Firstly find successor or predecessor of the provided. It will be
     * detached from the tree instead of the provided node and after that it
     * will replace the target node.
     */
    EntryBase *targetNode = node;
    EntryBase *replNode = node; /* Replacement node. */
    int dir; /* Initial direction. */
    if (replNode->child[0] || replNode->child[1]) {
        /* If there are children select direction. If there is a right item
         * which is red or there are no left item, then find predecessor.
         * Otherwise find successor.
         */
        if ((replNode->child[0] && replNode->child[0]->isRed) ||
            !replNode->child[1]) {
            dir = 0;
        } else {
            dir = 1;
        }
        replNode = replNode->child[dir];
        while (replNode->child[!dir]) {
            replNode = replNode->child[!dir];
        }
    }

    /* Re-balance the tree and detach replacement entry. */
    _RebalanceDeletion(replNode);

    ASSERT(_nodesCount);
    _nodesCount--;
    _generation++;

    /* Replace target entry with detached replacement entry. */
    if (replNode == targetNode) {
        /* The replacement entry which is also target entry was already
         * detached by the previous call, so we are done.
         */
        targetNode->isWired = false;
        return;
    }
    /* Move all links and color from target entry to the replacement one. */
    replNode->isRed = targetNode->isRed;
    targetNode->isWired = false;
    replNode->parent = targetNode->parent;
    if (targetNode->parent) {
        if (targetNode->parent->child[0] == targetNode) {
            targetNode->parent->child[0] = replNode;
        } else {
            ASSERT(targetNode->parent->child[1] == targetNode);
            targetNode->parent->child[1] = replNode;
        }
    } else {
        ASSERT(targetNode == _root);
        _root = replNode;
    }
    replNode->child[0] = targetNode->child[0];
    if (replNode->child[0]) {
        ASSERT(replNode->child[0]->parent == targetNode);
        replNode->child[0]->parent = replNode;
    }
    replNode->child[1] = targetNode->child[1];
    if (replNode->child[1]) {
        ASSERT(replNode->child[1]->parent == targetNode);
        replNode->child[1]->parent = replNode;
    }
}

RBTreeBase::EntryBase *
RBTreeBase::Lookup(void *key)
{
    EntryBase *node = _root;
    while (node) {
        int cmp = Compare(node, key);
        if (!cmp) {
            return node;
        }
        node = node->child[cmp > 0];
    }
    return 0;
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

bool
RBTreeBase::Validate()
{
    /* Iterate all nodes and check balancing rules validity for each node. */
    EntryBase *node = 0;
    int numBlackNodes = -1; /* Black nodes amount in a simple path. */
    while ((node = GetNextNode(node))) {
        /* Verify link with parent. */
        if (node->parent) {
            if (node->parent->child[0] != node && node->parent->child[1] != node) {
                /* Parent link broken. */
                return false;
            }
        }
        /* Validate children. */
        if (node->child[0] && Compare(node->child[0], node) >= 0) {
            return false;
        }
        if (node->child[1] && Compare(node->child[1], node) <= 0) {
            return false;
        }
        /* Red node never can have red children. */
        if (node->isRed && node->parent && node->parent->isRed) {
            return false;
        }
        /* If this is a leaf node, check black nodes amount in ascendant path. */
        if (!node->child[0] || !node->child[1]) {
            int n = 0;
            EntryBase *next = node;
            do {
                if (!next->isRed) {
                    n++;
                }
                next = next->parent;
            } while (next);
            if (numBlackNodes != -1) {
                if (numBlackNodes != n) {
                    return false;
                }
            } else {
                numBlackNodes = n;
            }
        }
    }
    return true;
}
