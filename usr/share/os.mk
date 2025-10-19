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
# These are mostly compiler flag and OS configurations,
# to configure the kernel go into /Makefile

SHELL := /bin/bash

# Put in your target, if cross compiling.
# Beware that Osiris currently supports only
# x86_64 and x86_64-elf-gcc could possibly break
# the build, since we include <limine.h> in many files.
CROSS_COMPILE ?= 

# Compile with clang by default.
# you can use gcc if you want, both are compatible with
# Osiris. Using a cross compiler is not required and discouraged,
# the system compiler should be enough.
# Beware that you absolutely SHOULD have GCC on your system
# since some Makefiles rely on it for a few tasks.
CC      := clang
AS      := nasm
LD      := ld
OBJCOPY := objcopy

# Compiler flags. LDFLAGS is not used for bin/ programs
# since they need special handling
CFLAGS	= -Wall -Wextra -ffreestanding -O0 -std=gnu11 -fno-stack-protector \
		 -fno-stack-check -fno-lto -fno-PIC -ffunction-sections \
		 -fdata-sections -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx \
		 -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -fno-omit-frame-pointer \
		 -g
CPPFLAGS	= -I include/ -I limine -DLIMINE_API_REVISION=3 -MMD -MP
LDFLAGS  = -nostdlib -static -m elf_x86_64  -z max-page-size=0x1000 \
		  --gc-sections -T sys/arch/x86_64/conf/kern.ld  
ASMFLAGS	= -f elf64

BUILD_DIR ?= ../build

DEPFLAGS := -MMD -MP -MF $(@:.o=.d)

# Define the programs you want to compile here.
PROGS=test