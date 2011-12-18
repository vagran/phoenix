# /phoenix/unit_tests/common/BuddyAllocator/Makefile
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

TEST_NAME = BuddyAllocator
TEST_DESC = Universal buddy allocator

TEST_SRCS = \
	$(PHOENIX_ROOT)/lib/common/BuddyAllocator.cpp \
	$(PHOENIX_ROOT)/lib/common/CommonLib.cpp \
	$(PHOENIX_ROOT)/lib/common/OTextStream.cpp \
	$(PHOENIX_ROOT)/lib/common/RBTree.cpp

TEST_DEFS = KERNEL

include $(PHOENIX_ROOT)/make/unit_test.mak
