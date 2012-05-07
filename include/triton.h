/*
 * /phoenix/include/triton.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file triton.h
 * Main include file for Phoenix C/C++ standard library - Triton library. It
 * provides most common classes which should be used by all Phoenix code such
 * as strings, lists, maps etc.
 */

#ifndef TRITON_H_
#define TRITON_H_

/** All Triton provided entities are defined in this namespace. */
namespace triton {

/** Transform type to a constant. */
template <typename T>
using add_const = const T;

/** Transform type to a reference. */
template <typename T>
using add_reference = T &;

/** Transform type to a constant reference. */
template <typename T>
using add_const_reference = const T &;

} /* namespace triton */

#include <triton/object.h>
#include <triton/exception.h>
#include <triton/ptr.h>
#include <triton/tuple.h>
#include <triton/iterator.h>

#endif /* TRITON_H_ */
