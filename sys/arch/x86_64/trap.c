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

/*
 * Trap handling for x86_64
 */

#include <stdint.h>
#include <liminefb.h>
#include <sys/portb.h>
#include <sys/printk.h>

void
infinite_loop ()
{
  uint64_t cr0;
  asm volatile ("mov %%cr0, %0" : "=r"(cr0));
  printk ("cr0=0x%llx", cr0);
  for (;;)
    {
      asm volatile ("cli");
      asm volatile ("hlt");
    }
}

void
isr0 ()
{
  liminefb_putstr ("Division by zero exception\n", 0xffffff);
  infinite_loop ();
}
void
isr1 ()
{
  liminefb_putstr ("Debug exception\n", 0xffffff);
  infinite_loop ();
}
void
isr2 ()
{
  liminefb_putstr ("Non Maskable Interrupt Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr3 ()
{
  liminefb_putstr ("Breakpoint Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr4 ()
{
  liminefb_putstr ("Into Detected Overflow Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr5 ()
{
  liminefb_putstr ("Out of Bounds Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr6 ()
{
  liminefb_putstr ("Invalid Opcode Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr7 ()
{
  liminefb_putstr ("No Coprocessor Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr8 ()
{
  liminefb_putstr ("Double Fault Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr9 ()
{
  liminefb_putstr ("Coprocessor Segment Overrun Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr10 ()
{
  liminefb_putstr ("Bad TSS Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr11 ()
{
  liminefb_putstr ("Segment Not Present Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr12 ()
{
  liminefb_putstr ("Stack Fault Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr13 ()
{
  liminefb_putstr ("General Protection Fault Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr14 ()
{
  liminefb_putstr ("Page Fault Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr15 ()
{
  liminefb_putstr ("Unknown Interrupt Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr16 ()
{
  liminefb_putstr ("Coprocessor Fault Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr17 ()
{
  liminefb_putstr ("Alignment Check Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr18 ()
{
  liminefb_putstr ("Machine Check Exception\n", 0xffffff);
  infinite_loop ();
}
void
isr_reserved ()
{
  liminefb_putstr ("Reserved (19-31) Exception\n", 0xffffff);
  infinite_loop ();
}

struct idt_entry
{
  uint16_t base_low;
  uint16_t sel;
  uint8_t always0;
  uint8_t flags;
  uint16_t base_mid;
  uint32_t base_high;
  uint32_t zero;
} __attribute__ ((packed));

struct idt_ptr
{
  uint16_t limit;
  uint64_t base;
} __attribute__ ((packed));

struct idt_entry idt[256];
struct idt_ptr idtptr;

void
fill_idt_entry (int num, uint64_t base, uint16_t sel, uint8_t flags)
{
  idt[num].base_low = (uint16_t)(base & 0xFFFF);
  idt[num].sel = sel;
  idt[num].always0 = 0;
  idt[num].flags = flags;
  idt[num].base_mid = (uint16_t)((base >> 16) & 0xFFFF);
  idt[num].base_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
  idt[num].zero = 0;
}

extern void do_isr0 ();
extern void do_isr1 ();
extern void do_isr2 ();
extern void do_isr3 ();
extern void do_isr4 ();
extern void do_isr5 ();
extern void do_isr6 ();
extern void do_isr7 ();
extern void do_isr8 ();
extern void do_isr9 ();
extern void do_isr10 ();
extern void do_isr11 ();
extern void do_isr12 ();
extern void do_isr13 ();
extern void do_isr14 ();
extern void do_isr15 ();
extern void do_isr16 ();
extern void do_isr17 ();
extern void do_isr18 ();

extern void do_irq0 ();
extern void do_irq1 ();

extern void load_idt ();

int ticks;

void
apit_irq ()
{
  extern int cursor_y, fb_height;
  ticks++;
  outb (0x20, 0x20);
  if (cursor_y > fb_height)
    {
      cursor_y = fb_height - 16;
    }
}

void
idt_init ()
{
  outb (0x20, 0x11);
  outb (0xA0, 0x11);
  outb (0x21, 0x20);
  outb (0xA1, 0x28);
  outb (0x21, 0x04);
  outb (0xA1, 0x02);
  outb (0x21, 0x01);
  outb (0xA1, 0x01);
  outb (0x21, 0x0);
  outb (0xA1, 0x0);

  fill_idt_entry (0, (uint64_t)do_isr0, 0x08, 0x8E);
  fill_idt_entry (1, (uint64_t)do_isr1, 0x08, 0x8E);
  fill_idt_entry (2, (uint64_t)do_isr2, 0x08, 0x8E);
  fill_idt_entry (3, (uint64_t)do_isr3, 0x08, 0x8E);
  fill_idt_entry (4, (uint64_t)do_isr4, 0x08, 0x8E);
  fill_idt_entry (5, (uint64_t)do_isr5, 0x08, 0x8E);
  fill_idt_entry (6, (uint64_t)do_isr6, 0x08, 0x8E);
  fill_idt_entry (7, (uint64_t)do_isr7, 0x08, 0x8E);
  fill_idt_entry (8, (uint64_t)do_isr8, 0x08, 0x8E);
  fill_idt_entry (9, (uint64_t)do_isr9, 0x08, 0x8E);
  fill_idt_entry (10, (uint64_t)do_isr10, 0x08, 0x8E);
  fill_idt_entry (11, (uint64_t)do_isr11, 0x08, 0x8E);
  fill_idt_entry (12, (uint64_t)do_isr12, 0x08, 0x8E);
  fill_idt_entry (13, (uint64_t)do_isr13, 0x08, 0x8E);
  fill_idt_entry (14, (uint64_t)do_isr14, 0x08, 0x8E);
  fill_idt_entry (15, (uint64_t)do_isr15, 0x08, 0x8E);
  fill_idt_entry (16, (uint64_t)do_isr16, 0x08, 0x8E);
  fill_idt_entry (17, (uint64_t)do_isr17, 0x08, 0x8E);
  fill_idt_entry (18, (uint64_t)do_isr18, 0x08, 0x8E);

  fill_idt_entry (32, (uint64_t)do_irq0, 0x08, 0x8E);
  fill_idt_entry (33, (uint64_t)do_irq1, 0x08, 0x8E);

  idtptr.limit = (sizeof (struct idt_entry) * 256) - 1;
  idtptr.base = (uint64_t)&idt;

  asm volatile ("lidt %0" : : "m"(idtptr));
}