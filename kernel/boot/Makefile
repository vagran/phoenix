# /phoenix/kernel/boot/Makefile
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

include $(PHOENIX_ROOT)/make/makevar.mak

LOADER_BINARY = phoenix.efi

COMPILE_DIR = build
OBJ_DIR = $(COMPILE_DIR)/$(PHOENIX_TARGET)

SUBDIRS = gnuefi lib lib/runtime lib/x86_64 loader \
	$(PHOENIX_ROOT)/lib/common $(PHOENIX_ROOT)/lib/elf

EFI_IMAGE = $(OBJ_DIR)/$(LOADER_BINARY)
EFI_DEBUG_IMAGE = $(OBJ_DIR)/debug.$(LOADER_BINARY)
ELF_IMAGE = $(OBJ_DIR)/elf_image.so

SRCS = $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.S $(dir)/*.c  $(dir)/*.cpp))

$(foreach dir,$(SUBDIRS),$(eval vpath %.S $(dir)) $(eval vpath %.c $(dir)) $(eval vpath %.cpp $(dir)))

OBJS = $(foreach src,$(SRCS), \
	$(OBJ_DIR)/$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir $(src))))))
DEPS = $(OBJS:.o=.d)

CFLAGS = $(GLOBAL_C_FLAGS) $(C_STANDARD) -Wno-pointer-sign

CPP_FLAGS = $(GLOBAL_CXX_FLAGS) $(CXX_STANDARD) $(CXX_RESTRICTIONS) \
	-fno-exceptions -fno-rtti

COMMON_FLAGS = $(GLOBAL_FLAGS) -fpic -Wall -Werror -fshort-wchar \
	-fno-strict-aliasing -fno-merge-constants -DEFI_FUNCTION_WRAPPER \
	-mno-red-zone -DCONFIG_x86_64 -DBOOT_LOADER -DEFI_APP

SECTIONS = .text .sdata .data .dynamic .dynsym .rel .rela .reloc
DEBUG_SECTIONS = .debug_info .debug_abbrev .debug_loc .debug_aranges \
	.debug_line .debug_macinfo .debug_str 

ifeq ($(PHOENIX_TARGET),RELEASE)
COMMON_FLAGS += -O2
else ifeq ($(PHOENIX_TARGET),DEBUG)
COMMON_FLAGS += -ggdb3 -O0 -DDEBUG -DEFI_DEBUG=1
else
$(error Target not supported: $(PHOENIX_TARGET))
endif

INCLUDE_DIRS = inc inc/protocol inc/x86_64 lib \
	$(PHOENIX_ROOT)/kernel/sys $(PHOENIX_ROOT)/kernel/sys/arch/$(PHOENIX_ARCH) \
	$(PHOENIX_ROOT)/include $(PHOENIX_ROOT)/lib/elf

IFLAGS = $(foreach incdir,$(INCLUDE_DIRS),-I$(incdir))

LD_SCRIPT = gnuefi/elf_x86_64_efi.lds

LDFLAGS = -nostdlib -T $(LD_SCRIPT) -shared -Bsymbolic --unresolved-symbols=report-all

.PHONY: all clean

all: $(EFI_IMAGE) $(EFI_DEBUG_IMAGE)

# include dependencies if exist
-include $(DEPS)

$(EFI_IMAGE): $(ELF_IMAGE)
	$(OBJCOPY) $(foreach sec,$(SECTIONS),-j $(sec)) --target=efi-app-x86_64 $< $@
	
$(EFI_DEBUG_IMAGE): $(ELF_IMAGE)
	$(OBJCOPY) $(foreach sec,$(SECTIONS) $(DEBUG_SECTIONS),-j $(sec)) --target=efi-app-x86_64 $< $@

$(ELF_IMAGE): $(OBJ_DIR) $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(OBJ_DIR)/%.o: %.S
	$(CC) -DCONFIG_x86_64 -c -o $@ $<
	
$(OBJ_DIR)/%.o: %.c
	$(CC) $(IFLAGS) $(COMMON_FLAGS) $(CFLAGS) -c -o $@ $<
	$(CC) -MM -MT '$@' $(IFLAGS) $(COMMON_FLAGS) $(CFLAGS) -c -o $(@:.o=.d) $<

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(IFLAGS) $(COMMON_FLAGS) $(CPP_FLAGS) -c -o $@ $<
	$(CC) -MM -MT '$@' $(IFLAGS) $(COMMON_FLAGS) $(CPP_FLAGS) -c -o $(@:.o=.d) $<

$(COMPILE_DIR):
	if [ ! -d $@ ]; then mkdir $@; fi

$(OBJ_DIR): $(COMPILE_DIR)
	if [ ! -d $@ ]; then mkdir $@; fi

clean:
	$(RM) -rf $(COMPILE_DIR)
