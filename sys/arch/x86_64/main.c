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

#include <atkbd.h>
#include <liminefb.h>
#include <sys/devfs/devfs_dev.h>
#include <sys/module.h>
#include <sys/panic.h>
#include <sys/portb.h>
#include <sys/vfs_mount.h>
#include <sys/printk.h>
#include <sys/string.h>
#include <sys/tar/tar_parse.h>
#include <x86_64/heap.h>
#include <x86_64/page.h>
#include <x86_64/request.h>
#include <x86_64/vmm/vmm_map.h>

extern void gdt_init ();
extern void trap_init ();
extern void tss_init ();
extern void proc_init ();

extern void mi_startup ();

void
x64_main ()
{
  gdt_init ();
  tss_init ();
  trap_init ();
  asm volatile ("cli");

  liminefb_init ();

  pmm_init ();

  vmm_init ();

  heap_init ();

  proc_init ();

  atkbd_init ();

  asm volatile("sti");

  /* Transfer control to the main startup function*/
  return mi_startup ();
}
