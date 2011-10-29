/*
 * /phoenix/kernel/sys/lock.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file lock.h
 * Machine independent classes for synchronization primitives implementation.
 */

#ifndef LOCK_H_
#define LOCK_H_

#include <md_lock.h>

template <size_t numTokens>
class Semaphore {
private:
    SpinLock lock;
    size_t _numTokens;
public:
    Semaphore() { _numTokens = 0; }

    /** Acquire resources.
     * @param tokens Number of tokens to reserve.
     */
    inline void Acquire(size_t tokens = 1)
    {
        ASSERT(tokens <= numTokens);
        bool acquired = false;
        do {
            lock.Lock();
            if (_numTokens + tokens <= numTokens) {
                _numTokens += tokens;
                acquired = true;
            }
            lock.Unlock();
            if (!acquired) {
                cpu::Pause();
            }
        } while (!acquired);
    }

    /** Release resources.
     * @param tokens Number of tokens to release.
     */
    inline void Release(size_t tokens = 1)
    {
        lock.Lock();
        ASSERT(tokens <= _numTokens);
        _numTokens -= tokens;
        lock.Unlock();
    }
};

#endif /* LOCK_H_ */
