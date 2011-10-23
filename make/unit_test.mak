# /phoenix/make/unit_test.mak
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

include $(PHOENIX_ROOT)/make/makevar.mak

COMPILE_DIR = $(CURDIR)/build
OBJ_DIR = $(COMPILE_DIR)/$(PHOENIX_TARGET)
UT_DIR = $(PHOENIX_ROOT)/unit_tests/ut

ifdef TEST_NAME
RMBUILD = $(RM) -rf $(COMPILE_DIR)
BINARY_NAME = $(OBJ_DIR)/$(TEST_NAME)
endif

SUBDIRS_TARGET = $(foreach item,$(SUBDIRS),$(item).dir)

LOCAL_SRCS = $(wildcard *.cpp)
UT_SRCS = $(notdir $(wildcard $(UT_DIR)/*.cpp))
vpath %.cpp $(UT_DIR)
SRCS = $(LOCAL_SRCS) $(UT_SRCS) $(notdir $(TEST_SRCS))

VPATH += $(dir $(TEST_SRCS))

OBJS = $(foreach obj,$(SRCS:.cpp=.o),$(OBJ_DIR)/$(obj))

INCLUDE_FLAGS += -I$(UT_DIR)

COMMON_FLAGS = -pipe -Wall -Werror
CXX_FLAGS = $(GLOBAL_FLAGS) $(GLOBAL_CXX_FLAGS) $(CXX_STANDARD) \
	$(CXX_RESTRICTIONS) $(TEST_CXX_FLAGS)

.PHONY: all clean $(SUBDIRS_TARGET) test

# make subdirectories if specified
ifdef SUBDIRS
$(SUBDIRS_TARGET):
	@$(MAKE) -C $(patsubst %.dir,%,$@) $(MAKECMDGOALS)
endif

all: $(BINARY_NAME) $(SUBDIRS_TARGET)

clean: $(SUBDIRS_TARGET)
	$(RMBUILD)

$(COMPILE_DIR):
	if [ ! -d $@ ]; then mkdir $@; fi

$(OBJ_DIR): $(COMPILE_DIR)
	if [ ! -d $@ ]; then mkdir $@; fi

$(BINARY_NAME): $(OBJ_DIR) $(OBJS)
	$(NAT_LD) -lstdc++ $(OBJS) -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(NAT_CC) -c $(INCLUDE_FLAGS) $(NAT_INCLUDE_FLAGS) $(COMMON_FLAGS) \
		$(CXX_FLAGS) -o $@ $<

ifdef TEST_NAME
test: $(BINARY_NAME)
	$(BINARY_NAME)
endif

test: $(SUBDIRS_TARGET)
