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

#include <stddef.h>
#include <stdint.h>
#include <osiris/kern/vfs_mount.h>

typedef struct devfs_node
{
  char name[12];
  fs_operations_t *ops;
  void *data;
  struct devfs_node *next;
} devfs_node_t;

extern devfs_node_t *devfs_root;

void devfs_init ();
void *devfs_lookup (void *fs_data, const char *name);
int devfs_read (char *node, void *buffer, int size);
int devfs_write (char *node, void *buffer, int size);

extern fs_operations_t devfs_ops;
