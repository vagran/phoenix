/*
 * /phoenix/lib/triton/list.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file list.cpp
 * Triton lists implementation.
 */

#include <sys.h>

using namespace triton;

void
triton_internal::ListBase::NodeBase::Link(NodeBase *prevNode)
{
    next = prevNode->next;
    next->prev = this;
    prev = prevNode;
    prev->next = this;
}

void
triton_internal::ListBase::NodeBase::Unlink()
{
    prev->next = next;
    next->prev = prev;
    next = this;
    prev = this;
}

void
triton_internal::ListBase::_ClearAll()
{
    /* Free all nodes. */
    NodeBase *node, *nextNode;
    for (node = _firstNode; node; node = nextNode) {
        if (node->next == _firstNode) {
            nextNode = nullptr;
        } else {
            nextNode = node->next;
        }
        _FreeNode(node);
    }
    _firstNode = nullptr;
    _numNodes = 0;
}

void
triton_internal::ListBase::_Append(NodeBase *node)
{
    if (_firstNode) {
        node->Link(_firstNode->prev);
    } else {
        _firstNode = node;
    }
    _numNodes++;
}

void
triton_internal::ListBase::_Insert(long idx, NodeBase *node)
{
    NodeBase *nextNode;
    if (idx > _numNodes || -idx > _numNodes) {
        nextNode = _firstNode;
    } else {
        nextNode = _GetNode(idx);
    }
    if (nextNode) {
        node->Link(nextNode->prev);
        if (-idx > _numNodes) {
            _firstNode = node;
        }
    } else {
        ASSERT(!_firstNode);
        _firstNode = node;
    }
    _numNodes++;
}

triton_internal::ListBase::NodeBase *
triton_internal::ListBase::_GetNode(long idx)
{
    if (idx >= _numNodes) {
        throw IndexError(/* "list index out of range" */);
    }
    if (idx < 0 && -idx > _numNodes) {
        throw IndexError(/* "list index out of range" */);
    }
    NodeBase *node = _firstNode;
    if (idx >= 0) {
        /* Traverse list forward. */
        while (idx > 0) {
            node = node->next;
            idx--;
        }
    } else {
        /* Traverse list backward. */
        while (idx < 0) {
            node = node->prev;
            idx++;
        }
    }
    return node;
}
