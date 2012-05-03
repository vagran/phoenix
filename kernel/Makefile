# /phoenix/kernel/Makefile
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

export KERNROOT = $(PHOENIX_ROOT)/kernel
MAKEIMAGE = 1

SUBDIRS = boot $(PHOENIX_ROOT)/lib/triton $(PHOENIX_ROOT)/lib/common \
	init efi kern vm

include $(KERNROOT)/make/make.mak
