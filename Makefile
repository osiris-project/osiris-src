#-
# SPDX-License-Identifier: 0BSD
#
# Copyright (c) 2025 V. Prokopenko
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#

include usr/share/os.mk

IMG          := $(BUILD_DIR)/osiris.iso
KERNEL_ELF   := $(BUILD_DIR)/bin/osiris.elf
FONT_OBJ     := $(BUILD_DIR)/font.o
SOURCE_DIR   := sys

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
C_SRCS     := $(call rwildcard,$(SOURCE_DIR)/,*.c)
ASM_SRCS   := $(call rwildcard,$(SOURCE_DIR)/,*.asm)
C_OBJS     := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRCS))
ASM_OBJS   := $(patsubst %.asm,$(BUILD_DIR)/%.o,$(ASM_SRCS))
OBJS       := $(C_OBJS) $(ASM_OBJS)

DEV_SUBDIRS := $(wildcard sys/dev/*)
CPPFLAGS    += $(patsubst %,-I%,$(DEV_SUBDIRS))

.PHONY: all kernel iso run font limine clean rebuild prune help check-deps clang-format
.DEFAULT_GOAL := help

help:
	@echo "!! Remember to check-deps before doing any compilation !!"
	@echo "Common targets are:"
	@echo "all"
	@echo "rebuild"
	@echo "iso"
	@echo "limine"
	@echo "clean"

# Build the / (root) directory for Osiris and
# pack the archive into world.tar
world:
	@echo "Making world..."
	@touch world.tar
	@$(MAKE) -C bin
	tar -cf world.tar bin/$(PROGS) usr/share/os.mk

all: $(IMG)
	qemu-system-x86_64 -cdrom $(IMG) -enable-kvm

kernel: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJS) $(FONT_OBJ)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(OBJS) $(FONT_OBJ) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASMFLAGS) $< -o $@

font:
	@mkdir -p $(dir $(FONT_OBJ))
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386 lib/*.psf $(FONT_OBJ)

limine:
	@git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1 || true
	@$(MAKE) -C limine

$(IMG): font limine kernel world
	@mkdir -p root/boot/limine root/EFI/BOOT
	@cp -v $(KERNEL_ELF) root/boot/osiris.elf
	@cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin \
		limine/limine-uefi-cd.bin root/boot/limine/
	@cp -v limine/BOOTX64.EFI root/EFI/BOOT/
	@cp -v limine/BOOTIA32.EFI root/EFI/BOOT/
	@cp -v world.tar root/boot/world.tar
	@xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		root -o $(IMG)
	@./limine/limine bios-install $(IMG)
	@./tools/generate_ksyms.sh

clang-format:
	find sys -name '*.c' -o -name '*.h' | xargs clang-format -i

rebuild:
	@$(MAKE) clean
	@$(MAKE) all

check-deps:
	@command -v xorriso >/dev/null 2>&1 || { echo "xorriso not installed"; exit 1; }
	@command -v $(CC) >/dev/null 2>&1 || { echo "$(CC) not installed"; exit 1; }
	@command -v $(AS) >/dev/null 2>&1 || { echo "$(AS) not installed"; exit 1; }
	@echo "All good!"

prune:
	@rm -f $(OBJS)

clean:
	rm -rf $(BUILD_DIR) limine $(IMG)
	$(MAKE) -C bin/ clean

-include $(C_OBJS:.o=.d)