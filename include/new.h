/*
 * /phoenix/include/new.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file new.h
 * Memory allocation operators.
 */

#ifndef NEW_H_
#define NEW_H_

/** @a new operator for untracked allocations. */
void *operator new(size_t size);

/** @a new operator for untracked arrays allocations. */
void *operator new[](size_t size);

/** @a new operator for untracked allocations with options.
 *
 * @param size Allocation size.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @param nonRec Memory allocation failure is not recoverable if @a true.
 */
void *operator new(size_t size, size_t align, bool nonRec = false);

/** @a new operator for untracked arrays allocations with options.
 *
 * @param size Allocation size.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @param nonRec Memory allocation failure is not recoverable if @a true.
 */
void *operator new[](size_t size, size_t align, bool nonRec = false);

/** @a new operator for tracked allocations with options.
 *
 * @param size Allocation size.
 * @param file Source file name where from the operator is invoked.
 * @param line Line number in the source file where from the operator is invoked.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @param nonRec Memory allocation failure is not recoverable if @a true.
 */
void *operator new(size_t size, const char *file, int line,
                   size_t align = 0, bool nonRec = false);

/** @a delete operator for non-arrays allocations. */
void operator delete(void *ptr);
/** @a delete operator for arrays allocations. */
void operator delete[](void *ptr);

#ifdef DEBUG
/** Macro to use operator @a new is required. It will ensure allocations tracking
 * in debug builds.
 */
#define NEW                     new(__FILE__, __LINE__)
/** Macro for allocating memory when the memory allocation failure is not
 * recoverable.
 */
#define NEW_NONREC              new(__FILE__, __LINE__, 0, true)
/** Macro for allocating aligned memory.
 * @param align Memory block alignment in bytes.
 */
#define NEW_ALIGNED(align)      new(__FILE__, __LINE__, align)
/** Macro for allocating memory with options. */
#define NEW_OPT(...)            new(__FILE__, __LINE__, ## __VA_ARGS__)

/** Macro for invoking @a delete operator. */
#define DELETE                  delete

#else /* DEBUG */

#define NEW                     new
#define NEW_NONREC              new(0, true)
#define NEW_ALIGNED(align)      new(align)
#define NEW_OPT(...)            new(__VA_ARGS__)

#define DELETE                  delete

#endif /* DEBUG */

#endif /* NEW_H_ */
