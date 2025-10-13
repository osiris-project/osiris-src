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

#include <stddef.h>
#include <stdint.h>

#include <x86_64/page.h>
#include <x86_64/vmm/vmm_map.h>
#include <sys/panic.h>
#include <sys/printk.h>
#include <sys/string.h>

/* These are the big, bad two functions for cloning page tables. They will get
 * used in fork(). They also only work for userspace mappings.
 *
 * Don't look into the code below. Even worse, don't even try to understand it.
 * It is pure insanity
 */

/* Free a cloned pagemap */
void
vmm_free_pagemap_clone (pagemap_t *clone)
{
  if (!clone || !clone->top_level)
    {
      printk ("vmm: vmm_free_pagemap_clone: invalid clone\n");
      return;
    }
  uint64_t *pml4 = clone->top_level;

  for (int pml4_i = 0; pml4_i < 512; ++pml4_i)
    {
      uint64_t pml4_e = pml4[pml4_i];
      if (!(pml4_e & PTE_PRESENT))
        continue;

      uintptr_t region_base = ((uintptr_t)pml4_i) << 39;
      if (region_base >= (uintptr_t)VMM_HIGHER_HALF)
        continue;

      uint64_t *pdpt = vmm_get_next_level (pml4, pml4_i, false, 0);
      if (!pdpt)
        continue;

      for (int pdpt_i = 0; pdpt_i < 512; ++pdpt_i)
        {
          uint64_t pdpt_e = pdpt[pdpt_i];
          if (!(pdpt_e & PTE_PRESENT))
            continue;

          uint64_t *pd = vmm_get_next_level (pdpt, pdpt_i, false, 0);
          if (!pd)
            continue;

          for (size_t pd_i = 0; pd_i < 512; ++pd_i)
            {
              uint64_t pd_e = pd[pd_i];
              if (!(pd_e & PTE_PRESENT))
                continue;

              uint64_t *pt = vmm_get_next_level (pd, pd_i, false, 0);
              if (!pt)
                continue;

              for (size_t pt_i = 0; pt_i < 512; ++pt_i)
                {
                  uint64_t pte = pt[pt_i];
                  if (!(pte & PTE_PRESENT))
                    continue;

                  uintptr_t phys = PTE_GET_ADDR (pte);
                  free_page ((void *)phys);
                  pt[pt_i] = 0;
                }

              uintptr_t pt_phys = PTE_GET_ADDR (pd_e);
              free_page ((void *)pt_phys);
              pd[pd_i] = 0;
            }

          uintptr_t pd_phys = PTE_GET_ADDR (pdpt_e);
          free_page ((void *)pd_phys);
          pdpt[pdpt_i] = 0;
        }

      uintptr_t pdpt_phys = PTE_GET_ADDR (pml4_e);
      free_page ((void *)pdpt_phys);
      pml4[pml4_i] = 0;
    }

  uintptr_t pml4_phys = (uintptr_t)pml4 - VMM_HIGHER_HALF;
  free_page ((void *)pml4_phys);
}

/* Clone a pagemap. Like I said before, it only works for userspace pagemaps */
pagemap_t *
vmm_clone_pagemap (pagemap_t *src)
{
  if (!src || !src->top_level)
    {
      printk ("vmm: vmm_clone_pagemap: invalid src\n");
      return NULL;
    }

  void *child_map_page_phys = allocate_page ();
  if (!child_map_page_phys)
    {
      printk ("vmm: vmm_clone_pagemap: failed to alloc child\n");
      return NULL;
    }
  pagemap_t *child_map
      = (pagemap_t *)((uintptr_t)child_map_page_phys + VMM_HIGHER_HALF);
  memset (child_map, 0, PAGE_SIZE);

  void *child_pml4_phys = allocate_page ();
  if (!child_pml4_phys)
    {
      printk ("vmm: vmm_clone_pagemap: failed to alloc pml4 child\n");
      free_page (child_map_page_phys);
      return NULL;
    }
  uint64_t *child_pml4
      = (uint64_t *)((uintptr_t)child_pml4_phys + VMM_HIGHER_HALF);
  memset (child_pml4, 0, PAGE_SIZE);

  child_map->top_level = child_pml4;

  uint64_t *src_pml4 = src->top_level;

  for (int pml4_i = 0; pml4_i < 512; ++pml4_i)
    {
      uint64_t src_pml4_e = src_pml4[pml4_i];
      if (!(src_pml4_e & PTE_PRESENT))
        continue;

      uintptr_t pml4_region_base = ((uintptr_t)pml4_i) << 39;

      if (pml4_region_base >= (uintptr_t)VMM_HIGHER_HALF)
        {
          child_pml4[pml4_i] = src_pml4_e;
          continue;
        }

      uint64_t *src_pdpt = vmm_get_next_level (src_pml4, pml4_i, false, 0);
      if (!src_pdpt)
        continue;

      void *child_pdpt_phys = allocate_page ();
      if (!child_pdpt_phys)
        goto clone_fail;
      uint64_t *child_pdpt
          = (uint64_t *)((uintptr_t)child_pdpt_phys + VMM_HIGHER_HALF);
      memset (child_pdpt, 0, PAGE_SIZE);
      child_pml4[pml4_i]
          = (uint64_t)(uintptr_t)child_pdpt_phys | (src_pml4_e & 0xFFF);

      for (int pdpt_i = 0; pdpt_i < 512; ++pdpt_i)
        {
          uint64_t src_pdpt_e = src_pdpt[pdpt_i];
          if (!(src_pdpt_e & PTE_PRESENT))
            continue;

          uint64_t *src_pd = vmm_get_next_level (src_pdpt, pdpt_i, false, 0);
          if (!src_pd)
            continue;

          void *child_pd_phys = allocate_page ();
          if (!child_pd_phys)
            goto clone_fail;
          uint64_t *child_pd
              = (uint64_t *)((uintptr_t)child_pd_phys + VMM_HIGHER_HALF);
          memset (child_pd, 0, PAGE_SIZE);
          child_pdpt[pdpt_i]
              = (uint64_t)(uintptr_t)child_pd_phys | (src_pdpt_e & 0xFFF);

          for (size_t pd_i = 0; pd_i < 512; ++pd_i)
            {
              uint64_t src_pd_e = src_pd[pd_i];
              if (!(src_pd_e & PTE_PRESENT))
                continue;

              uint64_t *src_pt = vmm_get_next_level (src_pd, pd_i, false, 0);
              if (!src_pt)
                continue;

              void *child_pt_phys = allocate_page ();
              if (!child_pt_phys)
                goto clone_fail;
              uint64_t *child_pt
                  = (uint64_t *)((uintptr_t)child_pt_phys + VMM_HIGHER_HALF);
              memset (child_pt, 0, PAGE_SIZE);
              child_pd[pd_i]
                  = (uint64_t)(uintptr_t)child_pt_phys | (src_pd_e & 0xFFF);

              for (size_t pt_i = 0; pt_i < 512; ++pt_i)
                {
                  uint64_t src_pte = src_pt[pt_i];
                  if (!(src_pte & PTE_PRESENT))
                    continue;

                  uintptr_t src_phys = PTE_GET_ADDR (src_pte);

                  void *child_page_phys = allocate_page ();
                  if (!child_page_phys)
                    goto clone_fail;

                  void *src_page_virt
                      = (void *)((uintptr_t)src_phys + VMM_HIGHER_HALF);
                  void *child_page_virt
                      = (void *)((uintptr_t)child_page_phys + VMM_HIGHER_HALF);
                  memcpy (child_page_virt, src_page_virt, PAGE_SIZE);

                  uint64_t flags = src_pte & 0xFFF;
                  child_pt[pt_i] = (uint64_t)(uintptr_t)child_page_phys | flags;
                }
            }
        }
    }

  return child_map;

clone_fail:
  printk ("vmm: vmm_clone_pagemap: clone_fail\n");
  vmm_free_pagemap_clone (child_map);

  free_page (child_map_page_phys);
  return NULL;
}