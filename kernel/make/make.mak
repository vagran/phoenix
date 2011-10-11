# /phoenix/kernel/make/make.mak
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

include $(PHOENIX_ROOT)/make/makevar.mak

# Common compile flags (all languages)
COMPILE_FLAGS = $(GLOBAL_FLAGS) -pipe -Werror -Wall -Wextra \
	-DKERNEL -fno-stack-protector -fno-builtin \
	-DLOAD_ADDRESS=$(KERNEL_LOAD_ADDRESS) \
	-DKERNEL_ADDRESS=$(KERNEL_ADDRESS)
COMPILE_FLAGS_C = $(GLOBAL_C_FLAGS) $(C_STANDARD)
COMPILE_FLAGS_CXX = $(GLOBAL_CXX_FLAGS) $(CXX_STANDARD) $(CXX_RESTRICTIONS) \
	-fno-exceptions -fno-rtti
COMPILE_FLAGS_ASM = -DASSEMBLER
LINK_FLAGS = -static -nodefaultlibs -nostartfiles -nostdinc -nostdinc++
LINK_SCRIPT = $(KERNROOT)/make/link.lds
LINK_MAP = $(OBJ_DIR)/kernel.map
AR_FLAGS = rcs

#PHOENIX_TARGET variable must be either DEBUG or RELEASE
ifndef PHOENIX_TARGET
TARGET = RELEASE
export PHOENIX_TARGET
endif

ifeq ($(PHOENIX_TARGET),RELEASE)
COMPILE_FLAGS += -O2
else ifeq ($(PHOENIX_TARGET),DEBUG)
COMPILE_FLAGS += -ggdb3 -DDEBUG -O0 -DENABLE_TRACING -DSERIAL_DEBUG -DDEBUG_MALLOC
else
$(error Target not supported: $(PHOENIX_TARGET))
endif

COMPILE_DIR = $(KERNROOT)/build
OBJ_DIR = $(COMPILE_DIR)/$(PHOENIX_TARGET)

SUBDIRS_TARGET = $(foreach item,$(SUBDIRS),$(item).dir)

ifdef MAKELIB
LIB_FILE = $(OBJ_DIR)/lib$(MAKELIB).a
endif

ifeq ($(DO_RAMDISK),1)
RAMDISK_SIZE = $(shell expr $(MFS_IMAGE_SIZE) '*' $(MFS_BLOCK_SIZE))
RAMDISK_FILE = $(MFS_IMAGE)
LINK_FILES += $(RAMDISK_FILE)
else
RAMDISK_SIZE = 0
endif

ifeq ($(MAKEIMAGE),1)
IMAGE = $(OBJ_DIR)/kernel
RMBUILD = $(RM) -rf $(COMPILE_DIR)
else
SRCS = $(wildcard *.S *.c *.cpp)
OBJS_LOCAL = $(subst .S,.o,$(subst .c,.o,$(subst .cpp,.o,$(SRCS))))
OBJS = $(foreach obj,$(OBJS_LOCAL),$(OBJ_DIR)/$(obj))
DEPS = $(OBJS:.o=.d)
endif

# Separate pre-linking for bootstrap objects
BOOT_SRCS = init.cpp
BOOT_OBJ_SRCS = $(foreach obj,$(BOOT_SRCS:.cpp=.o),$(OBJ_DIR)/$(obj))
BOOT_OBJ = $(OBJ_DIR)/boot.o
BOOT_LINK_SCRIPT = $(KERNROOT)/make/boot_link.lds
BOOT_LINK_MAP = $(OBJ_DIR)/boot.map

.PHONY: all clean FORCE $(SUBDIRS_TARGET)

all: $(OBJ_DIR) $(OBJS) $(IMAGE) $(SUBDIRS_TARGET) $(LIB_FILE)

# include dependencies if exist
-include $(DEPS)

ifeq ($(MAKEIMAGE),1)
$(BOOT_OBJ): $(BOOT_OBJ_SRCS) $(BOOT_LINK_SCRIPT)
	$(LD) $(LINK_FLAGS) -r -Map $(BOOT_LINK_MAP) -T $(BOOT_LINK_SCRIPT) -o $@ $^

$(IMAGE): $(OBJ_DIR) $(SUBDIRS_TARGET) $(LINK_SCRIPT) $(LINK_FILES) $(BOOT_OBJ)
	$(LD) $(LINK_FLAGS) --defsym LOAD_ADDRESS=$(KERNEL_LOAD_ADDRESS) \
		--defsym KERNEL_ADDRESS=$(KERNEL_ADDRESS) -Map $(LINK_MAP) \
		-T $(LINK_SCRIPT) -o $@ \
		$(filter-out $(BOOT_OBJ_SRCS) $(BOOT_OBJ), $(wildcard $(OBJ_DIR)/*.o)) \
		$(BOOT_OBJ) $(LINK_FILES)
endif

ifeq ($(DO_RAMDISK),1)
$(RAMDISK_FILE): FORCE
	@$(MAKE) -C $(PHOENIX_ROOT)/mfs $(RAMDISK_FILE)
endif

$(SUBDIRS_TARGET):
	@$(MAKE) -C $(patsubst %.dir,%,$@) $(MAKECMDGOALS)

$(COMPILE_DIR):
	if [ ! -d $@ ]; then mkdir $@; fi

$(OBJ_DIR): $(COMPILE_DIR)
	if [ ! -d $@ ]; then mkdir $@; fi
	cp $(TOOLS_ROOT)/gdb/gdbinit $@/.gdbinit
	cp $(TOOLS_ROOT)/gdb/phoenix_gdb.py $@/

$(OBJ_DIR)/%.o: %.c
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_C) -o $@ $<
	$(CC) -MM -MT '$@' -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) \
		$(COMPILE_FLAGS_C) -o $(@:.o=.d) $<

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_CXX) \
		$(subst $<,-DAUTONOMOUS_LINKING,$(findstring $<, $(BOOT_SRCS))) \
		-o $@ $<
	$(CC) -MM -MT '$@' -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) \
		$(COMPILE_FLAGS_CXX) -o $(@:.o=.d) $<

$(OBJ_DIR)/%.o: %.S
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_ASM) -o $@ $<

ifdef MAKELIB
$(LIB_FILE): $(OBJS)
	$(AR) $(AR_FLAGS) $@ $^
endif

clean: $(SUBDIRS_TARGET)
	$(RMBUILD)

install:
