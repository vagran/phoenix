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

/** This exception is thrown when Triton object null pointer is dereferenced. */
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

} /* namespace triton */

#endif /* EXCEPTION_H_ */
