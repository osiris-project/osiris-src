/*
 * Arikoto
 * Copyright (c) 2025 ㅤ
 * Licensed under the NCSA/University of Illinois Open Source License; see
 * LICENSE.md for details.
 */

#pragma once

#include <stddef.h>

void heap_init (void);

void *kmalloc (size_t size);

void kfree (void *ptr);

void *kcalloc (size_t num, size_t size);

void *krealloc (void *ptr, size_t size);
