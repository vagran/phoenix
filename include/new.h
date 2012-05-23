/*
 * /phoenix/include/new.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file new.h
 * Memory allocation operators.
 */

#ifndef NEW_H_
#define NEW_H_

/** Back-end for operator @a new which should be implemented by each environment.
 * This version is invoked in debug builds with source code location in
 * arguments.
 *
 * @param size Allocation size.
 * @param file Source file name where from the operator is invoked.
 * @param line Line number in the source file where from the operator is invoked.
 * @param isArray @a true if @a new[] operator was invoked, @a false otherwise.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @return Pointer to allocated memory block.
 */
void *__operator_new(size_t size, const char *file, int line,
                     bool isArray = false, size_t align = 0);

/** Back-end for operator @a new which should be implemented by each environment.
 * This version is invoked in release builds.
 *
 * @param size Allocation size.
 * @param isArray @a true if @a new[] operator was invoked, @a false otherwise.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @return Pointer to allocated memory block.
 */
void *__operator_new(size_t size, bool isArray = false, size_t align = 0);

/** Back-end for operator @a delete which should be implemented by each
 * environment.
 *
 * @param ptr Pointer to the memory block being freed.
 * @param isArray @a true if @a delete[] operator was invoked, @a false otherwise.
 */
void __operator_delete(void *ptr, bool isArray = false);

/** @a new operator for objects in-place construction.
 *
 * @param size Size of object.
 * @param location Pre-allocated area address where object should be constructed.
 */
inline void *
operator new(size_t size, void *location)
{
    return location;
}

/** @a new operator for untracked allocations.
 *
 * @param size Allocation size.
 * @return Pointer to allocated memory block.
 */
inline void *
operator new(size_t size)
{
    return __operator_new(size);
}

/** @a new operator for untracked arrays allocations.
 *
 * @param size Allocation size.
 * @return Pointer to allocated memory block.
 */
inline void *
operator new[](size_t size)
{
    return __operator_new(size, true);
}

/** @a new operator for untracked allocations with options.
 *
 * @param size Allocation size.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @return Pointer to allocated memory block.
 */
inline void *
operator new(size_t size, size_t align)
{
    return __operator_new(size, false, align);
}

/** @a new operator for untracked arrays allocations with options.
 *
 * @param size Allocation size.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @return Pointer to allocated memory block.
 */
inline void *
operator new[](size_t size, size_t align)
{
    return __operator_new(size, true, align);
}

/** @a new operator for tracked allocations with options.
 *
 * @param size Allocation size.
 * @param file Source file name where from the operator is invoked.
 * @param line Line number in the source file where from the operator is invoked.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @return Pointer to allocated memory block.
 */
inline void *
operator new(size_t size, const char *file, int line, size_t align = 0)
{
    return __operator_new(size, file, line, false, align);
}

/** @a new operator for tracked arrays allocations with options.
 *
 * @param size Allocation size.
 * @param file Source file name where from the operator is invoked.
 * @param line Line number in the source file where from the operator is invoked.
 * @param align Alignment value. Zero indicates that no specific alignment required.
 * @return Pointer to allocated memory block.
 */
inline void *
operator new[](size_t size, const char *file, int line, size_t align = 0)
{
    return __operator_new(size, file, line, true, align);
}

/** @a delete operator for non-arrays allocations.
 * @param ptr Pointer to memory block to free.
 */
inline void
operator delete(void *ptr)
{
    __operator_delete(ptr);
}

/** @a delete operator for arrays allocations.
 * @param ptr Pointer to memory block to free.
 */
inline void
operator delete[](void *ptr)
{
    __operator_delete(ptr, true);
}

#ifdef DEBUG
/** Macro to use operator @a new is required. It will ensure allocations tracking
 * in debug builds.
 */
#define NEW                     ::new(__FILE__, __LINE__)
/** Macro for allocating aligned memory.
 * @param align Memory block alignment in bytes.
 */
#define NEW_ALIGNED(align)      ::new(__FILE__, __LINE__, align)
/** Macro for allocating memory with options. */
#define NEW_OPT(...)            ::new(__FILE__, __LINE__, ## __VA_ARGS__)

/** Macro for invoking @a delete operator. */
#define DELETE                  ::delete

#else /* DEBUG */

#define NEW                     new
#define NEW_ALIGNED(align)      new(align)
#define NEW_OPT(...)            new(__VA_ARGS__)

#define DELETE                  delete

#endif /* DEBUG */

#endif /* NEW_H_ */
