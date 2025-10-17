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

#include <liminefb.h>
#include <stdint.h>
#include <sys/portb.h>
#include <sys/printk.h>

#include <liminefb.h>
#include <stdint.h>
#include <sys/portb.h>
#include <sys/printk.h>

typedef struct
{
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
  uint64_t int_no, err_code;
  uint64_t rip, cs, rflags, rsp, ss;
} registers_t;

extern void atkbd_irq ();
extern void schedule ();
int ticks = 0;

const char *trap_msgs[] = {
      "division by zero",
      "debug",
      "non maskable interrupt",
      "breakpoint",
      "into detected overflow",
      "out of bounds",
      "invalid opcode",
      "no coprocessor",
      "double fault",
      "coprocessor segment overrun",
      "bad tss",
      "segment not present",
      "stack fault",
      "general protection fault",
      "page fault",
      "unknown interrupt",
      "coprocessor fault",
      "alignment check",
      "machine check"
};

void
infinite_loop ()
{
  printk ("system halted\n");
  for (;;)
    {
      asm volatile ("cli");
      asm volatile ("hlt");
    }
}

void
isr_handler_c (registers_t *regs)
{
  if (regs->int_no < 19)
    {
      printk ("unhandled exception: %s (err code %llx)\n",
              trap_msgs[regs->int_no], regs->err_code);
    }
  else
    {
      printk ("reserved exc: %llx\n", regs->int_no);
    }
  infinite_loop ();
}

void
irq_handler_c (registers_t *regs)
{
  if (regs->int_no >= 40)
    {
      outb (0xA0, 0x20);
    }
  outb (0x20, 0x20);

  switch (regs->int_no)
    {
    case 32:
      ticks++;
      schedule ();
      break;
    case 33:
      atkbd_irq ();
      break;
    default:
      printk ("Unhandled IRQ: %llu\n", regs->int_no);
      break;
    }
}

/*
 * IDT setup code
 */
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

void
trap_init ()
{
  outb (0x20, 0x11);
  outb (0xA0, 0x11);
  outb (0x21, 0x20);
  outb (0xA1, 0x28);
  outb (0x21, 0x04);
  outb (0xA1, 0x02);
  outb (0x21, 0x01);
  outb (0xA1, 0x01);
  outb (0x21, 0x00);
  outb (0xA1, 0x00);

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