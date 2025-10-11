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

#include <osiris/arch/x86_64/heap.h>
#include <osiris/arch/x86_64/page.h>
#include <osiris/arch/x86_64/vmm/vmm_map.h>
#include <osiris/arch/x86_64/request.h>
#include <osiris/dev/atkbd.h>
#include <osiris/dev/liminefb.h>
#include <osiris/kern/panic.h>
#include <osiris/kern/portb.h>
#include <osiris/kern/printk.h>
#include <osiris/lib/string.h>

extern void gdt_init ();
extern void idt_init ();
extern void tss_init ();

extern void kernel_init ();

void
x64_main ()
{
  gdt_init ();
  tss_init ();
  idt_init ();

  liminefb_init ();
  printk("Osiris/x86_64 [text=0x%llx rodata=0x%llx data=0x%llx]\n", _text_end, _rodata_end, _data_end);

  pmm_init ();

  vmm_init ();

  heap_init ();
  
  /* Disable keyboard for now and turn on interrupts so PIT can start ticking */
  atkbd_disable ();
  asm volatile ("sti");

  atkbd_init ();

  /* Transfer control to the main init() function*/
  kernel_init ();
}
