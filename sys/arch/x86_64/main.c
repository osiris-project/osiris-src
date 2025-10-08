/*-
 * SPDX-License-Identifier: 0BSD
 *
 * Copyright (c) 2025 V. Prokopenko
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

#include <osiris/arch/x86_64/request.h>
#include <osiris/dev/atkbd.h>
#include <osiris/dev/liminefb.h>
#include <osiris/arch/x86_64/page.h>
#include <osiris/kern/panic.h>
#include <osiris/kern/portb.h>
#include <osiris/kern/printk.h>

extern void gdt_init ();
extern void idt_init ();
extern void tss_init ();
extern void pmm_init ();
extern void vmm_init ();

extern void kernel_init ();

const char *banner = "Osiris/x86_64 init\n";

void
x64_main ()
{
  gdt_init ();
  tss_init ();
  idt_init ();
  atkbd_disable ();
  asm volatile ("sti");
  liminefb_init ();
  printk (banner);
  atkbd_init ();
  pmm_init ();
  vmm_init ();
  /* Transfer control to the main init() function*/
  kernel_init ();
}
