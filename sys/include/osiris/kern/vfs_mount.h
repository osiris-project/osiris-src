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

#pragma once

#include <stdint.h>

typedef struct fs_operations
{
  void *(*lookup) (void *fs_data, const char *name);
  void *(*open) (char *path, int flags);
  int (*close) (void *fd);
  int (*read) (char *name, void *buffer, int count);
  int (*write) (char *name, void *buf, int size);
} fs_operations_t;

void vfs_mount (char *device, char *target, char *fs_type);
