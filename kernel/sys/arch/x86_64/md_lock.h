/*
 * /phoenix/kernel/sys/arch/x86_64/md_lock.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file md_lock.h
 * Machine dependent classes for synchronization primitives implementation.
 */

#ifndef MD_LOCK_H_
#define MD_LOCK_H_

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
    volatile u32 _flag;
public:
    inline SpinLock() { _flag = 0; }

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

/**
 * Read/write spin lock. It allows several simultaneous reads but doesn't allow
 * write simultaneously with another read or write.
 */
class RWSpinLock {
private:
    /* Higher bit indicates write lock. Lower bits store number of read locks
     * acquired or pending.
     */
    enum {
        WRITE_LOCK =    0x80000000,
        READ_LOCK =     0x7fffffff
    };

    volatile u32 _state;
public:
    inline RWSpinLock() { _state = 0; }
    inline ~RWSpinLock() { ASSERT(!_state); }

    /** Acquire read lock. Several simultaneous read locks can be acquired. */
    inline void ReadLock() {
        ASM(
            "lock incl %[state]\n" /* Create pending read. */
            :
            : [state]"m"(_state)
            : "cc");
        /* Wait until write lock released. */
        while (_state & WRITE_LOCK) {
            cpu::Pause();
        }
    }

    /** Release read lock. */
    inline void ReadUnlock() {
        ASSERT(_state & READ_LOCK);
        ASM(
            "lock decl %[state]\n" /* Decrement pending read. */
            :
            : [state]"m"(_state)
            : "cc");
    }

    /** Acquire write lock. It can be acquired only exclusively. It will not be
     * acquired after previous write lock released and there are pending read
     * locks.
     */
    inline void WriteLock() {
        u32 cur_state;
        ASM(
            /* Wait until there are no pending reads and write lock is not acquired. */
            "1: xorl %%eax, %%eax\n"
            "lock cmpxchgl %[write_lock], %[state]\n"
            "jz 2f\n"
            "pause\n"
            "jmp 1b\n"
            "2:\n"
            : "=a"(cur_state)
            : [state]"m"(_state), [write_lock]"r"(WRITE_LOCK)
            : "cc"
            );
    }

    /** Release write lock. */
    inline void WriteUnlock() {
        ASM(
            "lock andl %[write_lock], %[state]\n"
            :
            : [state]"m"(_state), [write_lock]"r"(~WRITE_LOCK)
            : "cc");
    }
};

#endif /* MD_LOCK_H_ */
