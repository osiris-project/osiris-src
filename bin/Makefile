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
# The bin/Makefile is a special case, since most programs
# here require their own .ld script. Try not to touch anything
# here since it works on my machine

include ../usr/share/os.mk

LDFLAGS  = -nostdlib -static -m64 \
           -Wl,-z,max-page-size=0x1000 \
           -Wl,--gc-sections

all: $(PROGS)

%: %.c
	@gcc $(CFLAGS) $(LDFLAGS) $< -o $* -Wl,-T,$*.ld

%: %.s
	$(AS) -f elf64 $< -o $@

clean:
	@rm -f $(PROGS)