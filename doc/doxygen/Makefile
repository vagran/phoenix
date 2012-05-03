# /phoenix/doc/doxygen/Makefile
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

include $(PHOENIX_ROOT)/make/makevar.mak

CONF_FILES = system_doc.conf

OUTPUT_DIRS = kernel

.PHONY: all clean test

clean:
	$(RM) -rf $(foreach dir,$(OUTPUT_DIRS),$(dir)/*)

all:
	for conf in $(CONF_FILES); do \
		$(DOXYGEN) $$conf; \
	done

test:
