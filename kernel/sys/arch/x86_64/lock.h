/*
 * /phoenix/kernel/sys/arch/x86_64/lock.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#ifndef LOCK_H_
#define LOCK_H_

/** @file lock.h
 * Machine dependent classes for synchronization primitives implementation.
 */

/** Memory barrier.
 *
 * Use it for placing memory barriers in the code. All loads and stores before
 * this operation are serialized and guaranteed to be globally visible after it.
 */
#define Barrier() { \
    ASM ("" ::: "memory"); \
    mfence(); \
}

/**
 * Spin lock synchronization primitive.
 *
 * This primitive just stops code execution in a small loop until a lock is
 * released.
 */
class SpinLock {
private:
    u32 _flag;
public:
    /**
     * @param isLocked Initial state of the lock.
     */
    inline SpinLock(bool isLocked = 0) { _flag = isLocked ? 1 : 0; }

    inline ~SpinLock() { ASSERT(!_flag); }

    /** Acquire lock. */
    inline void Lock() {
        ASM (
            "1: lock btsl $0, %[flag]\n"
            "jnc 2f\n"
            "pause\n"
            "jmp 1b\n"
            "2:\n"
            :
            : [flag]"m"(_flag)
            : "cc"
            );
    }

    /** Release lock. */
    inline void Unlock() {
        ASM (
            "lock btcl $0, %[flag]"
            :
            : [flag]"m"(_flag)
            : "cc"
            );
    }

    /** Try to acquire lock.
     *
     * This method will not block if the lock can not be acquired.
     *
     * @return 0 if successfully locked, -1 otherwise.
     */
    inline int TryLock() {
        register int rc;
        ASM (
            "xorl %%eax, %%eax\n"
            "lock btsl  $0, %[flag]\n"
            "jnc 1f\n"
            "movl $-1, %%eax\n"
            "1:\n"
            : "=&a"(rc)
            : [flag]"m"(_flag)
            : "cc"
            );
        return rc;
    }

    /**
     * @return Current state of the lock - true if locked, false if not locked.
     */
    inline operator bool() { return _flag ? true : false; }
};

#endif /* LOCK_H_ */
