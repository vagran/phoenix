/*
 * /phoenix/include/triton/exception.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file exception.h
 * Triton exceptions definition.
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

namespace triton {

/** Base class for all Triton exceptions. */
class TritonError: public Object {
private:
    /** Source file name where exception occurred. */
    const char *_file;
    /** Line number in the source file where exception occurred. */
    int _line;
public:
    TritonError() {}

    virtual
    ~TritonError() {}

    /** Name of this error. */
    virtual const char *
    __name__() const
    {
        return "TritonError";
    }

    //XXX __str__
    /** Detailed description of what had been occurred. */
    virtual const char *
    __desc__() const
    {
        return "Generic error";
    }
};

/** This exception is thrown when Triton object null pointer is dereferenced. */
class NullPtrError: public TritonError {
public:
    /** Name of this error. */
    virtual const char *
    __name__() const
    {
        return "NullPtrError";
    }

    //XXX __str__
    /** Detailed description of what had been occurred. */
    virtual const char *
    __desc__() const
    {
        return "Null-pointer dereferencing";
    }
};

/** This exception is thrown when Triton runs out of memory. */
class MemoryError: public TritonError {
public:
    /** Name of this error. */
    virtual const char *
    __name__() const
    {
        return "MemoryError";
    }

    //xxx __str__
    /** Detailed description of what had been occurred. */
    virtual const char *
    __desc__() const
    {
        //XXX allocation size info
        return "Memory allocation failure";
    }
};

/** The base class for the exceptions that are raised when a key or index used
 * on a mapping or sequence is invalid.
 */
class LookupError: public TritonError {
public:
    /** Name of this error. */
    virtual const char *
    __name__() const
    {
        return "LookupError";
    }

    //xxx __str__
};

/** Exception class which is raised when a sequence subscript is out of range. */
class IndexError: public LookupError {
public:
    /** Name of this error. */
    virtual const char *
    __name__() const
    {
        return "IndexError";
    }

    //xxx __str__
};

/** Exception class which is raised when a mapping (dictionary) key is not found
 * in the set of existing keys.
 */
class KeyError: public LookupError {
public:
    /** Name of this error. */
    virtual const char *
    __name__() const
    {
        return "KeyError";
    }

    //xxx __str__
};

} /* namespace triton */

#endif /* EXCEPTION_H_ */
