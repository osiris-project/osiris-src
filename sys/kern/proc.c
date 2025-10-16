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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/printk.h>
#include <sys/string.h>
#include <x86_64/heap.h>
#include <x86_64/vmm/vmm_map.h>

/* The Osiris scheduler */
/* Stack layout after a process is created: 
* high memory
* ========================
* |      stack top       | new_proc->stack + STACK_SIZE
* ========================
* | rip                  | first push
* ========================
* | r15                  |
* ========================
* | r14                  |
* ========================
* | r13                  |
* ========================
* | r12                  |
* ========================
* | r11                  |
* ========================
* | r10                  |
* ========================
* | r9                   |
* ========================
* | r8                   |
* ========================
* | rbp                  |
* ========================
* | rdi                  |
* ========================
* | rsi                  |
* ========================
* | rdx                  |
* ========================
* | rcx                  |
* ========================
* | rbx                  |
* ========================
* | rax                  |
* ========================
* low memory
*/

typedef enum
{
  PROC_READY,
  PROC_RUNNING,
  PROC_IDLE
} proc_state;

typedef struct Proc
{
  uint64_t rsp;
  uint64_t pid;
  proc_state state;
  uint64_t *stack;
} proc_t;

void
idle_proc ()
{
  for (;;)
    {
      asm volatile ("hlt");
    }
}

#define MAX_PROC 100    /* max proc count */
#define STACK_SIZE 4096 /* 4kb */

proc_t proc_table[MAX_PROC];
proc_t *current_proc;
int proc_count = 0;

extern void proc_switch_x64 (uint64_t *old_rsp_ptr, uint64_t new_rsp);

/* Main entry point */
void
schedule ()
{
  int current_proc_index = -1;
  for (int i = 0; i < proc_count; i++)
    {
      if (&proc_table[i] == current_proc)
        {
          current_proc_index = i;
          break;
        }
    }

  int next_proc_index = (current_proc_index + 1) % proc_count;
  while (proc_table[next_proc_index].state != PROC_READY
         && next_proc_index != current_proc_index)
    {
      next_proc_index = (next_proc_index + 1) % proc_count;
    }

  if (next_proc_index == current_proc_index)
    {
      return;
    }

  proc_t *old_proc = current_proc;
  proc_t *new_proc = &proc_table[next_proc_index];

  if (new_proc->state != PROC_READY)
    return;

  old_proc->state = PROC_READY;
  new_proc->state = PROC_RUNNING;
  current_proc = new_proc;

  proc_switch_x64 (&old_proc->rsp, new_proc->rsp);
}

/* Create a new process */
void
create_proc (void (*entry_point) ())
{
  /* TODO: actually handle this */
  if (proc_count >= MAX_PROC)
    {
      printk ("sched: max proc reached: %d\n", MAX_PROC);
      return;
    }

  proc_t *new_proc = &proc_table[proc_count];
  new_proc->pid = proc_count;
  new_proc->state = PROC_READY;

  new_proc->stack = (uint64_t *)kmalloc (STACK_SIZE);
  new_proc->rsp = (uint64_t)new_proc->stack + STACK_SIZE;

  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = (uint64_t)entry_point;

  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r15 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r14 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r13 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r12 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r11 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r10 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r9 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* r8 */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* rbi */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* rdi */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* rsi */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* rdx */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* rcx */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* rbx */
  new_proc->rsp -= 8;
  *(uint64_t *)new_proc->rsp = 0x0; /* rax */

  proc_count++;
}

/* Initialise the scheduler */
void
sched_init ()
{
  /* Clear the table */
  memset (proc_table, 0, sizeof (proc_table));
  /* The idle proc is a special case - we have to create it separately of
   * proc_create(). It also uses the kernel stack so there's no need to use
   * kmalloc */
  proc_t *idle = &proc_table[0];
  idle->pid = 0;
  idle->state = PROC_RUNNING;
  idle->stack = NULL;

  /* Set global state */
  proc_count = 1;
  current_proc = idle;
}