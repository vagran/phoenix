/*
 * /phoenix/unit_tests/common/BuddyAllocator/test.cpp
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

#include <phoenix_ut.h>

#include <sys.h>

UT_TEST("Universal buddy allocator")
{
    BuddyAllocator<vm::Vaddr> alloc;
    const int maxOrder = 20;
    const size_t poolSize = 11 * (1 << maxOrder);
    const size_t managedSize = poolSize - (1 << maxOrder);
    u8 *pool = new u8[poolSize];
    pool = vm::Vaddr((vm::Vaddr(pool) + (1 << maxOrder) - 1) & ~((1 << maxOrder) - 1));

    bool rc = OK(alloc.Initialize(pool, pool + managedSize, 0, maxOrder));
    UT(rc) == UT(true);
}
UT_TEST_END
