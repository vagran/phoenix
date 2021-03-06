# /phoenix/make/make.mak
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011-2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

include $(PHOENIX_ROOT)/make/makevar.mak

ifdef KERNROOT
# Compile the component as a part of the kernel
include ${KERNROOT}/make/make.mak
else

ifndef LOAD_ADDRESS
LOAD_ADDRESS = $(DEF_LOAD_ADDRESS)
endif

# Common compile flags (all languages)
COMPILE_FLAGS += $(GLOBAL_FLAGS) -pipe -Werror -Wall -Wextra 
	-fno-stack-protector -fno-builtin -DKERNEL_ADDRESS=$(KERNEL_ADDRESS)
COMPILE_FLAGS_C +=  $(GLOBAL_C_FLAGS) $(C_STANDARD)
COMPILE_FLAGS_CXX +=  $(GLOBAL_CXX_FLAGS) -fno-exceptions -fno-rtti \
	$(CXX_STANDARD) $(CXX_RESTRICTIONS)
COMPILE_FLAGS_ASM += -DASSEMBLER
LINK_FLAGS += -nodefaultlibs -nostartfiles -nostdinc -nostdinc++ \
	--no-omagic -z common-page-size=0x1000 --defsym LOAD_ADDRESS=$(LOAD_ADDRESS)
	
COMPILE_FLAGS += $(foreach inc,$(INCLUDES),-I$(inc))

#PHOENIX_TARGET variable must be either DEBUG or RELEASE
ifndef PHOENIX_TARGET
export PHOENIX_TARGET = RELEASE
endif

########
ifdef APP

ifndef LINK_SCRIPT
ifeq ($(STATIC),1)
LINK_SCRIPT = $(PHOENIX_ROOT)/make/link.app.lds
else
LINK_SCRIPT = $(PHOENIX_ROOT)/make/link.dynapp.lds
endif
endif

ifneq ($(NO_INSTALL),1)
ifndef INSTALL_DIR
INSTALL_DIR = /bin
endif
endif

BINARY_NAME = $(APP)
export PROFILE_NAME = APP
IS_PROFILE_ROOT = 1
LINK_FILES += $(APP_RUNTIME_LIB)

ifeq ($(STATIC),1)
LINK_FLAGS += -Bstatic
LINK_FILES += $(COMMON_LIB)
LINK_FILES += $(foreach lib,$(LIBS),$(PHOENIX_ROOT)/lib/$(lib)/build/$(PHOENIX_TARGET)/lib$(lib).a)
else
LINK_FLAGS += -Bdynamic
LINK_FILES += $(subst .a,.sl,$(COMMON_LIB)
LINK_FILES += $(foreach lib,$(LIBS),$(PHOENIX_ROOT)/lib/$(lib)/build/$(PHOENIX_TARGET)/lib$(lib).sl)
endif

ifeq ($(EXPORTABLE),1)
LINK_FLAGS += -E
endif

REQUIRE_RUNTIME_LIB = 1

########
else ifdef OBJ
BINARY_NAME = $(OBJ)
export PROFILE_NAME = OBJ
IS_PROFILE_ROOT = 1

LINK_FLAGS += -r

########
else ifdef LIB

ifndef LINK_SCRIPT
LINK_SCRIPT = $(PHOENIX_ROOT)/make/link.lib.lds
endif

BINARY_NAME = lib$(LIB).a

ifneq ($(STATIC),1)
LINK_FLAGS += -Bdynamic -init __sl_initialize -fini __sl_finalize
BINARY_NAME += lib$(LIB).sl
LINK_FILES += $(SL_RUNTIME_LIB) 
LINK_FILES += $(foreach lib,$(LIBS),$(PHOENIX_ROOT)/lib/$(lib)/build/$(PHOENIX_TARGET)/lib$(lib).sl)
endif

ifeq ($(EXPORTABLE),1)
LINK_FLAGS += -E
endif

export PROFILE_NAME = LIB
IS_PROFILE_ROOT = 1

ifneq ($(NO_INSTALL),1)
ifndef INSTALL_DIR
INSTALL_DIR = $(LIBS_INSTALL_DIR)
endif
endif

########
else ifndef SUBDIRS
$(error Build profile not specified)
endif

ifdef IS_PROFILE_ROOT
export PROFILE_ROOT = $(CURDIR)
RMBUILD = $(RM) -rf $(COMPILE_DIR)
endif

ifdef PROFILE_NAME
COMPILE_DIR = $(PROFILE_ROOT)/build
OBJ_DIR = $(COMPILE_DIR)/$(PHOENIX_TARGET)
endif

ifdef BINARY_NAME
BINARY = $(foreach bin,$(BINARY_NAME),$(OBJ_DIR)/$(bin))
endif

ifeq ($(MODULE_TESTS),y)
COMPILE_FLAGS += -DMODULE_TESTS
endif

ifeq ($(PHOENIX_TARGET),RELEASE)
COMPILE_FLAGS += -O2
else ifeq ($(PHOENIX_TARGET),DEBUG)
COMPILE_FLAGS += -ggdb3 -DDEBUG -O0 -DENABLE_TRACING \
	-DSERIAL_DEBUG -DDEBUG_MALLOC
else
$(error Target not supported: $(PHOENIX_TARGET))
endif

SRCS += $(wildcard *.S *.c *.cpp)
OBJS_LOCAL = $(subst .S,.o,$(subst .c,.o,$(subst .cpp,.o,$(SRCS))))
OBJS = $(foreach obj,$(OBJS_LOCAL),$(OBJ_DIR)/$(obj))
OBJS_SO = $(OBJS:.o=.so)
DEPS = $(OBJS:.o=.d)

SUBDIRS_TARGET = $(foreach item,$(SUBDIRS),$(item).dir)

ifdef LINK_SCRIPT
LINK_FLAGS += -T $(LINK_SCRIPT)
endif

ifdef LINK_SCRIPTS
LINK_FLAGS += $(foreach script,$(LINK_SCRIPTS), -T $(script))
endif

.PHONY: all clean install test FORCE $(SUBDIRS_TARGET) $(BINARY_NAME)

all: $(BINARY) $(SUBDIRS_TARGET)

# include dependencies if exist
-include $(DEPS)

ifdef BINARY
# Phony file name targets depends on absolute paths of the files
$(BINARY_NAME): % : $(OBJ_DIR)/%

# Archive files for static linkage
$(filter %.a, $(BINARY)): $(SUBDIRS_TARGET) $(OBJ_DIR) $(OBJS)
	$(AR) rcs $@ $(OBJS)
	
# Shared library
$(filter %.sl, $(BINARY)): $(SUBDIRS_TARGET) $(OBJ_DIR) $(LINK_SCRIPT) \
	$(LINK_SCRIPTS) $(LINK_FILES) $(OBJS_SO)
	$(LD) $(LINK_FLAGS) -fpic -shared -o $@ \
	--start-group $(LINK_FILES) $(OBJS_SO) --end-group \
	-soname=$(@F)

# Relocatable static object file
$(filter %.o, $(BINARY)): $(SUBDIRS_TARGET) $(OBJ_DIR) $(LINK_FILES) $(OBJS)
	$(LD) $(LINK_FLAGS) -o $@ \
	--start-group $(LINK_FILES) $(OBJS) --end-group

# Relocatable dynamic object file
$(filter %.so, $(BINARY)): $(SUBDIRS_TARGET) $(OBJ_DIR) $(LINK_FILES) $(OBJS_SO)
	$(LD) $(LINK_FLAGS) -o $@ \
	--start-group $(LINK_FILES) $(OBJS_SO) --end-group

# The executable binary
$(filter-out %.a %.sl %.o %.so, $(BINARY)): $(SUBDIRS_TARGET) $(OBJ_DIR) $(LINK_SCRIPT) \
	$(LINK_SCRIPTS) $(LINK_FILES) $(OBJS)
	$(LD) $(LINK_FLAGS) -o $@ --start-group $(LINK_FILES) $(OBJS) --end-group \
	--dynamic-linker $(RT_LINKER_DIR)/$(RT_LINKER_NAME)
endif

# Build dependencies - static and dynamic libraries
$(filter %.a %.sl %.o %.so, $(LINK_FILES)):
	$(MAKE) -C $(abspath $(@D)/../..) $(@F)

# Relocatable objects
$(OBJ_DIR)/%.o: %.c
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_C) -o $@ $<
	$(CC) -MM -MT '$@' -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) \
		$(COMPILE_FLAGS_C) -o $(@:.o=.d) $<

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_CXX) -o $@ $<
	$(CC) -MM -MT '$@' -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) \
		$(COMPILE_FLAGS_CXX) -o $(@:.o=.d) $<

$(OBJ_DIR)/%.o: %.S
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_ASM) -o $@ $<

PIC_FLAGS += -fpic -D__PIC=1

# Position-independent objects
$(OBJ_DIR)/%.so: %.c
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_C) $(PIC_FLAGS) -o $@ $<
	$(CC) -MM -MT '$@' -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) \
		$(COMPILE_FLAGS_C) $(PIC_FLAGS) -o $(@:.o=.d) $<

$(OBJ_DIR)/%.so: %.cpp
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_CXX) $(PIC_FLAGS) -o $@ $<
	$(CC) -MM -MT '$@' -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) \
		$(COMPILE_FLAGS_CXX) $(PIC_FLAGS) -o $(@:.o=.d) $<

$(OBJ_DIR)/%.so: %.S
	$(CC) -c $(INCLUDE_FLAGS) $(COMPILE_FLAGS) $(COMPILE_FLAGS_ASM) $(PIC_FLAGS) -o $@ $<

$(SUBDIRS_TARGET):
	@$(MAKE) -C $(patsubst %.dir,%,$@) $(MAKECMDGOALS)

$(COMPILE_DIR):
	[ -d $@ ] || mkdir $@

$(OBJ_DIR): $(COMPILE_DIR)
	[ -d $@ ] || mkdir $@
	cp $(TOOLS_ROOT)/gdb/gdbinit $@/.gdbinit
	cp $(TOOLS_ROOT)/gdb/phoenix_gdb.py $@/

ifdef IS_PROFILE_ROOT
clean: $(SUBDIRS_TARGET)
	$(RMBUILD)
else
clean: $(SUBDIRS_TARGET)
endif

ifdef INSTALL_DIR
INSTALLED_BINARY = $(filter-out %.a %.o, $(BINARY))
ifneq ($(INSTALLED_BINARY),)
install: $(INSTALLED_BINARY) $(SUBDIRS_TARGET)
	$(foreach bin,$(INSTALLED_BINARY),$(INSTALL) -D $(bin) \
		$(INSTALL_ROOT)/$(PHOENIX_TARGET)$(INSTALL_DIR)/$(notdir $(bin));)
endif
else
install: $(SUBDIRS_TARGET)
endif

test: $(SUBDIRS_TARGET)

# KERNROOT
endif
