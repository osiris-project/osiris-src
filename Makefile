# 
#
# Osiris Makefile
#
# The common targets, which are all you probably need are:
#
# all		- Build the kernel sources and iso, then run using qemu.
#
# kernel	- Build only the kernel image. This won't install the bootloader or
#             the iso image. You won't be able to run this though. Should be 
#             enough to just see if the kernel compiles during debugging.
#
# 
# font 		- Convert .psf font files to raw object files.
#
# limine 	- Compile limine.
#
# clean 	- Do clean up. Removes the vendor directories, font and 
#             kernel binaries.
# rebuild 	- Delete all binaries and recompile the kernel.
#
# prune		- Remove all object files but keep the final binary
#
# Other targets should be self-explanatory, or targeted at the developer.
#

# Detect the number of cores and use all of them during the build. Shouldn't be 
# a problem as the kernel is pretty small. However, there have been some race
# conditions with Limine compilation so turn these on at your own risk. 1 
# modern thread should be enough anyway.
# NUM_CORES ?= $(shell nproc)
# MAKEFLAGS += -j$(NUM_CORES)

SHELL := /bin/bash

# Name of the image we're going to build
IMG	= build/osiris.iso

# Right now, since the kernel has no other targets than x86_64, we're going
# to use the default GCC toolchain. There's no need for a cross compiler.
# This should be changed if not compiling on x86_64 though
#
# Don't get a x86_64 cross compiler for this kind of job, the system one
# is enough.
CC	= gcc
LD	= ld
AS	= nasm
OBJCOPY	= objcopy

# Compiler flags.
CFLAGS	= -Wall -Wextra -ffreestanding -O0 -std=gnu11 -fno-stack-protector \
		 -fno-stack-check -fno-lto -fno-PIC -ffunction-sections \
		 -fdata-sections -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx \
		 -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -fno-omit-frame-pointer \
		 -g
CPPFLAGS	= -I include/ -I limine -DLIMINE_API_REVISION=3 -MMD -MP
LDFLAGS  = -nostdlib -static -m elf_x86_64  -z max-page-size=0x1000 \
		  --gc-sections -T sys/arch/x86_64/conf/kern.ld  
ASMFLAGS	= -f elf64


SOURCE_DIR	= sys
BUILD_DIR	?= build

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
C_SRCS = $(call rwildcard,sys/,*.c)
ASM_SRCS = $(call rwildcard,sys/,*.asm)
C_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRCS))
ASM_OBJS = $(patsubst %.asm,$(BUILD_DIR)/%.o,$(ASM_SRCS))

DEV_SUBDIRS := $(wildcard sys/dev/*)
CPPFLAGS += $(patsubst %,-I%,$(DEV_SUBDIRS))

OBJS = $(C_OBJS) $(ASM_OBJS)

KERNEL_ELF = build/bin/osiris.elf

.PHONY: all iso run font limine clean kernel clang-format help rebuild check-deps

.DEFAULT_GOAL := help

help:
	@echo "!! Remember to check-deps before doing any compilation !!"
	@echo "Common targets are:"
	@echo "all"
	@echo "rebuild"
	@echo "iso"
	@echo "limine"
	@echo "clean"
	@echo "Refer to the Makefile comment header for explanations"

all: $(IMG)
	qemu-system-x86_64 -cdrom build/osiris.iso -enable-kvm

ksyms:
	touch include/sys/ksyms.h

kernel: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJS) sys/arch/x86_64/conf/kern.ld 
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(OBJS) build/font.o -o $@

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	mkdir -p $(dir $@)
	$(AS) $(ASMFLAGS) $< -o $@

$(IMG): ksyms $(BUILD_DIR) font limine $(KERNEL_ELF)
	@# Do basic checks to prevent any errors.
	@$(shell command -v xorriso >/dev/null 2>&1 || { echo "xorriso not installed"; exit 1; })
	@$(shell command -v gcc >/dev/null 2>&1 || { echo "gcc not installed"; exit 1; })
	@$(shell command -v nasm >/dev/null 2>&1 || { echo "nasm not installed"; exit 1; })

	@mkdir -p usr/boot/limine
	@touch usr/boot/initrd.tar
	@tar -cf usr/boot/initrd.tar rootfs/README.txt
	@mkdir -p usr/EFI/BOOT
	@cp -v $(KERNEL_ELF) usr/boot/osiris.elf
	@cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin \
		limine/limine-uefi-cd.bin usr/boot/limine/
	@cp -v limine/BOOTX64.EFI usr/EFI/BOOT/
	@cp -v limine/BOOTIA32.EFI usr/EFI/BOOT/
	@xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        usr -o $(IMG)
	@./limine/limine bios-install $(IMG)
	./tools/generate_ksyms.sh

limine:
	git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1 || true
	$(MAKE) -C limine

#!!! THERE SHOULD ONLY BE ONE FONT IN LIB/ !!!
# I could hardcode this, but I intend for you to be able to use
# your own 8x16 font. So delete the original font if you want to
# use your own.
font:
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386 lib/*.psf $(BUILD_DIR)/font.o

clang-format:
	find sys -name '*.c' -o -name '*.h' | xargs clang-format -i

$(BUILD_DIR):
	mkdir build

rebuild:
	@$(MAKE) clean
	@$(MAKE) all

check-deps:
	@$(shell command -v xorriso >/dev/null 2>&1 || { echo "xorriso not installed"; exit 1; })
	@$(shell command -v gcc >/dev/null 2>&1 || { echo "gcc not installed"; exit 1; })
	@$(shell command -v nasm >/dev/null 2>&1 || { echo "nasm not installed"; exit 1; })
	@echo "All good!"

prune:
	@rm $(OBJS)

clean:
	rm -rf build usr limine $(IMG)

-include $(C_OBJS:.o=.d)
