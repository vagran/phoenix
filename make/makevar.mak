# /phoenix/make/make.mak
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

TOOLS_BIN = $(PHOENIX_BUILD_TOOLS)/bin
TOOLS_PREFIX = x86_64-phoenix-elf-
TOOLS_SUFFIX = -4.7.0

export CC = $(TOOLS_BIN)/$(TOOLS_PREFIX)gcc$(TOOLS_SUFFIX)
export LD = $(TOOLS_BIN)/$(TOOLS_PREFIX)ld
export STRIP = $(TOOLS_BIN)/$(TOOLS_PREFIX)strip
export AR = $(TOOLS_BIN)/$(TOOLS_PREFIX)ar
export OBJCOPY = $(TOOLS_BIN)/$(TOOLS_PREFIX)objcopy
export INSTALL = install
export DD = dd
export CP = cp
export RM = rm
export MOUNT = mount
export UMOUNT = umount
export DF = df
export CAT = cat
export SUDO_ASKPASS = /usr/bin/ssh-askpass
export SUDO = sudo
export MKE2FS = mke2fs
export DOXYGEN = $(TOOLS_BIN)/doxygen

export NAT_CC = $(TOOLS_BIN)/gcc
export NAT_LD = $(TOOLS_BIN)/gcc
export NAT_NM = $(TOOLS_BIN)/nm
export NAT_CPPFILT = $(TOOLS_BIN)/c++filt

export APP_RUNTIME_LIB_NAME = apprt
export APP_RUNTIME_LIB_DIR = $(PHOENIX_ROOT)/lib/startup
export APP_RUNTIME_LIB = $(APP_RUNTIME_LIB_DIR)/build/$(PHOENIX_TARGET)/lib$(APP_RUNTIME_LIB_NAME).a

export SL_RUNTIME_LIB_NAME = slrt.so
export SL_RUNTIME_LIB_DIR = $(PHOENIX_ROOT)/lib/sl_startup
export SL_RUNTIME_LIB = $(SL_RUNTIME_LIB_DIR)/build/$(PHOENIX_TARGET)/$(SL_RUNTIME_LIB_NAME)

export COMMON_LIB_DIR = $(PHOENIX_ROOT)/lib/common
export COMMON_LIB_NAME = common
export COMMON_LIB = $(COMMON_LIB_DIR)/build/$(PHOENIX_TARGET)/lib$(COMMON_LIB_NAME).a

export RT_LINKER_DIR = /bin
export RT_LINKER_NAME = rt_linker

export LIBS_INSTALL_DIR = /usr/lib

export INSTALL_ROOT = $(PHOENIX_ROOT)/install

export TOOLS_ROOT = $(PHOENIX_ROOT)/tools

# Default load address for binary executables
export DEF_LOAD_ADDRESS = 0x1000

INCLUDE_DIRS = $(PHOENIX_ROOT)/kernel $(PHOENIX_ROOT)/kernel/sys \
	$(PHOENIX_ROOT)/kernel/sys/arch/$(PHOENIX_ARCH) $(PHOENIX_ROOT)/include 
INCLUDE_FLAGS += $(foreach dir,$(INCLUDE_DIRS),-I$(dir))

NAT_INCLUDE_DIRS = 
NAT_INCLUDE_FLAGS = $(foreach dir,$(NAT_INCLUDE_DIRS),-I$(dir))

GLOBAL_FLAGS = -DARCH_$(PHOENIX_ARCH)
GLOBAL_C_FLAGS =
GLOBAL_CXX_FLAGS =

C_STANDARD = -std=c99
CXX_STANDARD = -std=c++0x

CXX_RESTRICTIONS = -Wold-style-cast -Woverloaded-virtual

# kernel physical load address
export KERNEL_LOAD_ADDRESS =	0x00200000
# kernel virtual mapping address
export KERNEL_ADDRESS =	0x78000000

MFS_IMAGE_NAME = mfs_image
MFS_IMAGE = $(PHOENIX_ROOT)/mfs/image/$(PHOENIX_TARGET)/$(MFS_IMAGE_NAME)
MFS_BLOCK_SIZE = 1024
MFS_IMAGE_LABEL = Phoenix_MFS
# MFS image size in kilobytes
MFS_IMAGE_SIZE = 8000
