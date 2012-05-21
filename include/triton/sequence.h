/*
 * /phoenix/include/triton/sequence.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file sequence.h
 * Triton sequence classes definition.
 */

#ifndef SEQUENCE_H_
#define SEQUENCE_H_

namespace triton {

template <typename T>
class Iterable {
    //XXX
};

template <typename T>
class Sequence: public Container<T> {
public:
    typedef typename Container<T>::ValueType ValueType;

    /** Type of index of value in a sequence. */
    typedef long index_t;

    /** Get minimal value in a sequence. @ref ValueError is raised if the
     * sequence is empty.
     * @return Minimal value.
     */
    virtual ValueType &
    __min__() = 0;

    /** Get maximal value in a sequence. @ref ValueError is raised if the
     * sequence is empty.
     * @return Maximal value.
     */
    virtual ValueType &
    __max__() = 0;

    /** Get value at specified position. Negative values are counted from the
     * sequence end. In case the index is out of the sequence range the @ref
     * IndexError is thrown.
     *
     * @param idx Index of value to retrieve.
     * @return Value at specified position.
     */
    virtual ValueType &
    operator [](index_t idx) = 0;

    /** Get index of the first occurrence of @a value in the sequence. @ref
     * ValueError is raised if @a value is not present in a sequence.
     *
     * @param value Value to search.
     * @return Index of the first occurrence of @a value.
     */
    virtual index_t
    index(T &&value) = 0;

    /** Get total number of occurrences of @a value in the sequence.
     *
     * @param value Value to search.
     * @return Total number of occurrences of @a value.
     */
    virtual size_t
    count(T &&value) = 0;

    /** Append value to the end of the sequence.
     *
     * @param value Value to append.
     */
    virtual void
    append(T &&value) = 0;

    /** Extend the sequence by values from provided iterable object. Values are
     * appended to the end of the sequence.
     *
     * @param it Iterable to get values from.
     */
    virtual void
    extend(Iterable<T> &it) = 0;

    /** Insert value in the sequence at specified position.
     *
     * @param idx Position index to insert value to. If it is greater than size
     *      of the sequence the value is appended to the end. If the index is
     *      negative it is counted from the end of the sequence.
     * @param value Value to insert.
     */
    virtual void
    insert(index_t idx, T &&value) = 0;
};

} /* namespace triton */

#endif /* SEQUENCE_H_ */
