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
 * write simultaneously with another read or write. @n
 *
 * Several fields are defined:
 * @li Pending write (PWR) - this flag indicates that write lock is pending.
 *      The flag can be set only if there are no WR flag set.
 * @li Write locked (WR) - this flag indicates that write lock is currently
 *      acquired. It can be set only if RD field is zero and PWR is set. PWR
 *      is cleared when write lock is acquired.
 * @li Read lock (RD) - field which is a counter either of currently acquired
 *      read locks (if WR is not set) or currently pending read locks (if WR is
 *      set). This field can be incremented only if PWR is not set.
 *
 */
class RWSpinLock {
private:
    /* Higher bits indicate write lock acquisition and pending. Lower bits store
     * number of read locks acquired or pending.
     */
    enum {
        WRITE_LOCK =    0x80000000,
        WRITE_PENDING = 0x40000000,
        READ_LOCK =     0x3fffffff
    };

    volatile u32 _state;
public:
    inline RWSpinLock() { _state = 0; }
    inline ~RWSpinLock() { ASSERT(!_state); }

    /** Acquire read lock. Several simultaneous read locks can be acquired. */
    inline void ReadLock() {
        ASM(
            /* Wait until PWR is not set. */
            "movl %[state], %%eax\n"
            "1:\n"
            "andl %[notPwr], %%eax\n"
            "movl %%eax, %%edx\n"
            "incl %%edx\n" /* Increment pending or acquired reads counter */
            "lock cmpxchgl %%edx, %[state]\n"
            "jz 2f\n" /* Reads counter incremented */
            "pause\n"
            "jmp 1b\n"
            "2:\n"
            :
            : [state]"m"(_state),
              [notPwr]"i"(~WRITE_PENDING)
            : "cc", "eax", "edx");

        /* Wait until write lock released. No atomicity required because
         * pending read is non-zero so new write lock can not be acquired
         * until read locks are released.
         */
        while (_state & WRITE_LOCK) {
            cpu::Pause();
        }
    }

    /** Release read lock. */
    inline void ReadUnlock() {
        ASSERT(_state & READ_LOCK);
        ASSERT(!(_state & WRITE_LOCK));
        ASM(
            "lock decl %[state]\n" /* Decrement acquired read. */
            :
            : [state]"m"(_state)
            : "cc");
    }

    /** Acquire write lock. It can be acquired only exclusively. It will not be
     * acquired after previous write lock released and there are pending read
     * locks.
     */
    inline void WriteLock() {
        ASM(
            /* Set PWR flag. This can be done only if WR is cleared. */
            "movl %[state], %%eax\n"
            "jmp 3f\n"
            "1:\n"
            "andl %[notWr], %%eax\n" /* WR should be cleared */
            "movl %%eax, %%edx\n"
            "orl %[pwr], %%edx\n"
            "lock cmpxchgl %%edx, %[state]\n"
            "jz 2f\n" /* PWR was set */
            "pause\n"
            "jmp 1b\n"
            "2:\n"
            /* Wait until there are no pending reads and check that PWR is set. */
            "movl %%edx, %%eax\n"
            "3:\n"
            "testl %[pwr], %%eax\n"
            "jz 1b\n" /* PWR is not set (other CPU acquired write lock), roll back to the previous loop */
            "andl %[notRd], %%eax\n" /* Read locks counter should be zero */
            "movl %%eax, %%edx\n"
            "orl %[wr], %%edx\n" /* Set WR */
            "andl %[notPwr], %%edx\n" /* Clear PWR */
            "lock cmpxchgl %%edx, %[state]\n"
            "jz 4f\n"
            "pause\n"
            "jmp 3b\n"
            "4:\n"
            :
            : [state]"m"(_state),
              [wr]"i"(WRITE_LOCK),
              [notWr]"i"(~WRITE_LOCK),
              [pwr]"i"(WRITE_PENDING),
              [notPwr]"i"(~WRITE_PENDING),
              [notRd]"i"(~READ_LOCK)
            : "cc", "eax", "edx"
            );
    }

    /** Release write lock. */
    inline void WriteUnlock() {
        ASSERT(_state & WRITE_LOCK);
        ASM(
            "lock andl %[notWr], %[state]\n"
            :
            : [state]"m"(_state),
              [notWr]"r"(~WRITE_LOCK)
            : "cc");
    }
};

#endif /* MD_LOCK_H_ */
