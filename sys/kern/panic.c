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
 * This file contains the kernel debugger (odb) and the panic() function itself.
 */
#include <stddef.h>
#include <stdint.h>

#include <atkbd.h>
#include <sys/printk.h>

struct ksym
{
  uint64_t addr;
  const char *name;
};

struct ksym ksyms[] = {
#include "sys/ksyms.h"
};

size_t ksyms_count = sizeof (ksyms) / sizeof (ksyms[0]);

const struct ksym *
odb_addr_to_sym (uint64_t rip)
{
  int l = 0, r = ksyms_count - 1;
  const struct ksym *last = NULL;
  while (l <= r)
    {
      int m = (l + r) / 2;
      if (rip >= ksyms[m].addr)
        {
          last = &ksyms[m];
          l = m + 1;
        }
      else
        {
          r = m - 1;
        }
    }
  return last;
}

void
odb_stack_trace ()
{
  uint64_t *rbp;
  asm volatile ("mov %%rbp, %0" : "=r"(rbp));

  printk ("odb: stack trace:\n", 0xffffff);
  int i = 0;
  while (rbp && (uint64_t)rbp < 0xffffffffa0000000)
    {
      i++;
      uint64_t rip = rbp[1];
      const struct ksym *sym = odb_addr_to_sym (rip);
      if (sym)
        {
          uint64_t offset = rip - sym->addr;
          printk ("  #%d [0x%llx]: at %s+0x%d\n", i, rip, sym->name, offset);
        }
      else
        {
          return;
        }
      rbp = (uint64_t *)rbp[0];
    }
}

typedef struct registers
{
  uint64_t rax, rbx, rcx, rdx;
  uint64_t rsi, rdi, rbp, rsp;
  uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
  uint64_t rflags;
} registers_t;

registers_t regs;

void
odb_read_registers (registers_t *regs)
{
  asm volatile ("mov %%rax, %0" : "=m"(regs->rax));
  asm volatile ("mov %%rbx, %0" : "=m"(regs->rbx));
  asm volatile ("mov %%rcx, %0" : "=m"(regs->rcx));
  asm volatile ("mov %%rdx, %0" : "=m"(regs->rdx));
  asm volatile ("mov %%rsi, %0" : "=m"(regs->rsi));
  asm volatile ("mov %%rdi, %0" : "=m"(regs->rdi));
  asm volatile ("mov %%rbp, %0" : "=m"(regs->rbp));
  asm volatile ("mov %%rsp, %0" : "=m"(regs->rsp));
  asm volatile ("mov %%r8,  %0" : "=m"(regs->r8));
  asm volatile ("mov %%r9,  %0" : "=m"(regs->r9));
  asm volatile ("mov %%r10, %0" : "=m"(regs->r10));
  asm volatile ("mov %%r11, %0" : "=m"(regs->r11));
  asm volatile ("mov %%r12, %0" : "=m"(regs->r12));
  asm volatile ("mov %%r13, %0" : "=m"(regs->r13));
  asm volatile ("mov %%r14, %0" : "=m"(regs->r14));
  asm volatile ("mov %%r15, %0" : "=m"(regs->r15));
  asm volatile ("pushfq\n\t"
                "popq %0"
                : "=m"(regs->rflags));
}

void
odb_dump_registers (registers_t *regs)
{
  printk ("RAX:0x%llx RBX:0x%llx RCX:0x%llx RDX:0x%llx\n", regs->rax, regs->rbx,
          regs->rcx, regs->rdx);
  printk ("RSI:0x%llx RDI:0x%llx RBP:0x%llx RSP:0x%llx\n", regs->rsi, regs->rdi,
          regs->rbp, regs->rsp);
  printk ("R8 :0x%llx R9 :0x%llx R10:0x%llx R11:0x%llx\n", regs->r8, regs->r9,
          regs->r10, regs->r11);
  printk ("R12:0x%llx R13:0x%llx R14:0x%llx R15:0x%llx\n", regs->r12, regs->r13,
          regs->r14, regs->r15);
  printk ("FLAGS:0x%llx\n", regs->rflags);
}

void
cpuid (uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
       uint32_t *edx)
{
  asm volatile ("cpuid"
                : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                : "a"(leaf));
}

void
odb_cpuid ()
{
  uint32_t eax, ebx, ecx, edx;
  char vendor[13];

  cpuid (0, &eax, &ebx, &ecx, &edx);

  *((uint32_t *)&vendor[0]) = ebx;
  *((uint32_t *)&vendor[4]) = edx;
  *((uint32_t *)&vendor[8]) = ecx;
  vendor[12] = '\0';
  printk ("cpuid = %s\n", vendor);
}

/* The main ODB function */
void
odb_enter ()
{
  odb_cpuid ();
  extern int ticks;
  printk ("uptime=%d\n", ticks);
  odb_dump_registers (&regs);
  odb_stack_trace ();
}

void
panic (const char *fmt)
{
  odb_read_registers (&regs);
  asm volatile ("cli");
  printk ("panic: %s\n", fmt);
  odb_enter ();
  printk ("FATAL: halting system\n");
  for (;;)
    {
      asm volatile ("cli");
      asm volatile ("hlt");
    }
}