# /phoenix/kernel/boot/Makefile
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

include $(PHOENIX_ROOT)/make/makevar.mak

LOADER_BINARY = phoenix.efi

COMPILE_DIR = build
OBJ_DIR = $(COMPILE_DIR)/$(TARGET)

SUBDIRS = gnuefi lib lib/runtime lib/x86_64 loader

EFI_IMAGE = $(OBJ_DIR)/$(LOADER_BINARY)
ELF_IMAGE = $(OBJ_DIR)/elf_image.so

SRCS = $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.S $(dir)/*.c))

$(foreach dir,$(SUBDIRS),$(eval vpath %.S $(dir)) $(eval vpath %.c $(dir)))

OBJS = $(foreach src,$(SRCS),$(OBJ_DIR)/$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir $(src)))))

CFLAGS = -O2 -fpic -Wall -Werror -fshort-wchar -fno-strict-aliasing \
	-fno-merge-constants -DEFI_FUNCTION_WRAPPER -mno-red-zone -DCONFIG_x86_64 \
	-DBOOT_LOADER
	
INCLUDE_DIRS = inc inc/protocol inc/x86_64 lib $(PHOENIX_ROOT)/kernel/sys
IFLAGS = $(foreach incdir,$(INCLUDE_DIRS),-I$(incdir))

LD_SCRIPT = gnuefi/elf_x86_64_efi.lds

LDFLAGS = -nostdlib -T $(LD_SCRIPT) -shared -Bsymbolic

.PHONY: all clean

all: $(EFI_IMAGE)

$(EFI_IMAGE): $(ELF_IMAGE)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel \
		-j .rela -j .reloc --target=efi-app-x86_64 $< $@

$(ELF_IMAGE): $(OBJ_DIR) $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	
$(OBJ_DIR)/%.o: %.S
	$(CC) -DCONFIG_x86_64 -c -o $@ $<
	
$(OBJ_DIR)/%.o: %.c
	$(CC) $(IFLAGS) $(CFLAGS) -c -o $@ $<

$(COMPILE_DIR):
	if [ ! -d $@ ]; then mkdir $@; fi

$(OBJ_DIR): $(COMPILE_DIR)
	if [ ! -d $@ ]; then mkdir $@; fi

clean:
	$(RM) -rf $(COMPILE_DIR)
