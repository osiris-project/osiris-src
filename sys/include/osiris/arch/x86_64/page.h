/*
 * Arikoto
 * Copyright (c) 2025 ㅤ
 * Licensed under the NCSA/University of Illinois Open Source License; see
 * LICENSE.md for details.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Initialize the physical memory manager */
void pmm_init (void);

/* Allocate a single page (PAGE_SIZE bytes) of physical memory */
void *allocate_page (void);

/* Free a previously allocated page of physical memory */
void free_page (void *page);

/* Get total size of manageable physical memory in bytes */
size_t get_total_memory (void);

/* Get used physical memory size in bytes */
size_t get_used_memory (void);

/* Get free physical memory size in bytes */
size_t get_free_memory (void);

/* Check if a specific physical page is free (used internally by VMM) */
bool is_page_free (uintptr_t paddr);

/* Helper to align values up */
#ifndef ALIGN_UP
#define ALIGN_UP(value, align) (((value) + (align) - 1) & ~((align) - 1))
#endif
