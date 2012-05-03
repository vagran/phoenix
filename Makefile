# /phoenix/Makefile
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

SUBDIRS = doc kernel unit_tests

include $(PHOENIX_ROOT)/make/make.mak

clean:
	rm -rf $(INSTALL_ROOT)
	#$(MAKE) -C mfs clean
