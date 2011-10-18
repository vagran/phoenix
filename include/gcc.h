/*
 * /phoenix/include/gcc.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file gcc.h
 * GCC low-level run-time support code.
 */

#ifndef GCC_H_
#define GCC_H_

#ifdef __cplusplus

/** C++ ABI support class. */
class Cxa {
public:
    typedef void    (*CbkFunc)();
    struct CallbackList {
        u32     n;
        CbkFunc func[1];
    };

    static int ConstructStaticObjects();
    static int DestructStaticObjects();

private:
    struct Destructor {
        void    (*func)(void *);
        void    *arg;
        void    *dso_handle;
    };
};

#endif /* __cplusplus */

/* FIXME probably these function not required for 64 bits platform */
ASMCALL i64 __divdi3(i64 a, i64 b);
ASMCALL u64 __udivdi3(u64 a, u64 b);
ASMCALL i64 __moddi3(i64 a, i64 b);
ASMCALL u64 __umoddi3(u64 a, u64 b);
ASMCALL u64 __qdivrem(u64 u, u64 v, u64 *rem);

#endif /* GCC_H_ */
