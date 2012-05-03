/*
 * /phoenix/include/RetCode.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file RetCode.h
 * Definition of the class used as return value to indicate successful or
 * failed call.
 */

#ifndef RETCODE_H_
#define RETCODE_H_

/** Class used as return value to indicate successful or failed call. */
class RetCode {
public:
    /** Values used to indicate success or error. */
    enum Code {
        SUCCESS, /**< The call fully succeeded. */
        FAIL, /**< Generic failure. */
        INV_PARAM, /**< Provided parameter(s) is(are) invalid. */
        NOT_FOUND, /**< Requested object not found. */
        NO_MEMORY, /**< Failed to allocate memory. */
        NO_RESOURCES, /**< Out of required resources. */
    };

    /** Construct RetCode object. Should not be used directly - use @ref RC
     * macro instead.
     *
     * @param retCode Status code to return.
     */
    inline RetCode(Code retCode = SUCCESS) {
        code = retCode;
    }

    /** Cast the object to numeric code value. */
    inline operator Code() { return code; }

    /** Check if the status is successful. */
    inline bool IsOk() { return code == SUCCESS; }

    /** Check if the status indicates failure. */
    inline bool IsFailed() { return code != SUCCESS; }

    const char *GetName() {
        switch (code) {
        case SUCCESS:
            return "SUCCESS";
        case FAIL:
            return "FAIL";
        case INV_PARAM:
            return "INV_PARAM";
        case NOT_FOUND:
            return "NOT_FOUND";
        case NO_MEMORY:
            return "NO_MEMORY";
        case NO_RESOURCES:
            return "NO_RESOURCES";
        }
        return "UNKNOWN";
    }

    inline operator const char *() {
        return GetName();
    }

    Code code;
};

#ifdef DEBUG

/** Macro for constructing return code object. This should be used instead of
 * of direct calls to @ref RetCode class constructor in order to have additional
 * troubleshooting functionality in debug builds.
 * @param __code Return code - unqualified name of @ref RetCode::Code enum member.
 * @return @ref RetCode class object.
 */
#define RC(__code)      ({ \
    if (RetCode::__code != RetCode::SUCCESS) { \
        TRACE("Function '%s' at %s:%d failed: %s", \
              __func__, __FILE__, __LINE__, __STR(__code)); \
    } \
    RetCode::__code; \
})

/** Check if the return code is successful. */
#define OK(__rc)        ({ \
    RetCode __Xrc = __rc; \
    if ((__Xrc).IsFailed()) { \
        TRACE("Failed return code received in '%s' at %s:%d: %s", \
              __func__, __FILE__, __LINE__, (__Xrc).GetName()); \
    } \
    (__Xrc).IsOk(); \
})

/** Check if the return code indicates failure. */
#define NOK(__rc)       ({ \
    RetCode __Xrc = __rc; \
    if ((__Xrc).IsFailed()) { \
        TRACE("Failed return code received in '%s' at %s:%d: %s", \
              __func__, __FILE__, __LINE__, (__Xrc).GetName()); \
    } \
    (__Xrc).IsFailed(); \
})

#else /* DEBUG */

#define RC(__code)      RetCode(RetCode::__code)

#define OK(__rc)        (LIKELY((__rc).IsOk()))

#define NOK(__rc)       (UNLIKELY((__rc).IsFailed()))

#endif /* DEBUG */

#endif /* RETCODE_H_ */
