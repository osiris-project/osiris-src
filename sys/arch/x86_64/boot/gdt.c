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

/* This file has no header for it, as the only thing we really need is
 * gdt_init() and we could just extern it in the init.c */
/* GDT initialisation is an essential part of startup on x86_64. There aren't
 * any ports planned to other architectures, and the kernel is in a pretty early
 * stage so we'll just gdt_init() in the main function. */
#include <stdint.h>

__attribute__ ((aligned (16))) uint8_t kernel_stack[16384];

__attribute__ ((aligned (16))) uint8_t user_stack[65536];

__attribute__ ((aligned (16))) uint8_t df_stack[65536];

__attribute__ ((aligned (16))) uint8_t nmi_stack[65536];

struct __attribute__ ((packed)) TSS
{
  uint32_t reserved0;
  uint64_t rsp0;
  uint64_t rsp1, rsp2;
  uint64_t reserved1;
  uint64_t ist[7];
  uint64_t reserved2;
  uint16_t reserved3, io_map_base;
} tss;

typedef struct
{
  uint16_t limit;
  uint64_t base;
} __attribute__ ((packed)) gdt_register;

typedef struct
{
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__ ((packed)) gdt_entry;

gdt_entry gdt[7];

void
gdt_fill_entry (int num, uint8_t access, uint8_t granularity, uint32_t base,
                uint32_t limit)
{
  gdt[num].limit_low = limit & 0xFFFF;
  gdt[num].base_low = base & 0xFFFF;
  gdt[num].base_mid = (base >> 16) & 0xFF;
  gdt[num].access = access;
  gdt[num].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
  gdt[num].base_high = (base >> 24) & 0xFF;
}

void
gdt_set_tss (int num, uint64_t base, uint32_t limit)
{
  gdt_fill_entry (num, 0x89, 0x00, base, limit);
  uint32_t *hi = (uint32_t *)&gdt[num + 1];
  hi[0] = base >> 32;
  hi[1] = 0;
}

gdt_register gdt_reg;

void
gdt_flush ()
{
  asm volatile ("mov $0x10, %%ax\n"
                "mov %%ax, %%ds\n"
                "mov %%ax, %%es\n"
                "mov %%ax, %%fs\n"
                "mov %%ax, %%gs\n"
                "mov %%ax, %%ss\n"
                "pushq $0x08\n"
                "lea 1f(%%rip), %%rax\n"
                "pushq %%rax\n"
                "lretq\n" /* This returns */
                "1:\n"
                :
                :
                : "rax");
}

/* This must be called AFTER gdt_init, otherwise it'll triple fault. We do IST
 * stuff with the GDT to not split this up into even more functions*/
void
tss_init ()
{
  asm volatile ("ltr %%ax" ::"a"(0x18));
}

void
gdt_init ()
{
  gdt_fill_entry (0, 0, 0, 0, 0);
  gdt_fill_entry (1, 0x9A, 0x20, 0, 0);
  gdt_fill_entry (2, 0x92, 0x00, 0, 0);
  gdt_set_tss (3, (uint64_t)&tss, sizeof (tss) - 1);
  gdt_fill_entry (5, 0xFA, 0x20, 0, 0);
  gdt_fill_entry (6, 0xF2, 0x00, 0, 0);

  /* Configure the TSS IST, exceptions might overwrite the kernel stack so we
   * need another one.*/
  tss.ist[0] = (uint64_t)(df_stack + sizeof (df_stack));
  tss.ist[1] = (uint64_t)(nmi_stack + sizeof (nmi_stack));

  tss.rsp0 = (uint64_t)(kernel_stack + sizeof (kernel_stack));

  gdt_reg.limit = sizeof (gdt) - 1;
  gdt_reg.base = (uint64_t)&gdt;
  asm volatile ("lgdt %0" ::"m"(gdt_reg));
  /* This will return from the function, so don't place anything after this
   * part. */
  gdt_flush ();
}