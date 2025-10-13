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

#include <osiris/arch/x86_64/heap.h>
#include <osiris/arch/x86_64/request.h>
#include <osiris/fs/devfs/devfs_dev.h>
#include <osiris/fs/tar/tar_parse.h>
#include <osiris/kern/module.h>
#include <osiris/kern/panic.h>
#include <osiris/kern/printk.h>
#include <osiris/kern/vfs_mount.h>
#include <osiris/lib/strcmp.h>
#include <osiris/lib/string.h>

#define VFS_TYPE_LENGTH 32
#define VFS_PATH_LENGTH 64
#define MAX_MOUNTPOINTS 12

typedef struct mountpoint
{
  char type[VFS_TYPE_LENGTH];
  char device[VFS_PATH_LENGTH];
  char mountpoint[VFS_PATH_LENGTH];
  fs_operations_t operations;

  struct mountpoint *next;
} mountpoint_t;

mountpoint_t *mountpoints_root;

/* Create a mountpoint. fs_operations_t is left uninitialised since we do that
 * during vfs_mount
 */
mountpoint_t *
vfs_create_mountpoint (char *device, char *target, char *type)
{
  mountpoint_t *mp = (mountpoint_t *)kmalloc (sizeof (mountpoint_t));
  if (!mp)
    {
      printk ("vfs: mountpoint_t: failed to alloc for mountpoint\n");
      return NULL;
    }

  kstrcpy (mp->device, device);
  kstrcpy (mp->mountpoint, target);
  kstrcpy (mp->type, type);
  mp->next = NULL;

  return mp;
}

/* Add a mountpoint to the linked list*/
mountpoint_t *
vfs_add_mountpoint (mountpoint_t *mp)
{
  if (!mountpoints_root)
    {
      mountpoints_root = mp;
      return mp;
    }

  mountpoint_t *cur = mountpoints_root;
  while (cur->next)
    cur = cur->next;
  cur->next = mp;
  return mp;
}

/* Remove a mountpoint and all associated stuff */
void
vfs_remove_mountpoint (mountpoint_t *mp)
{
  if (!mountpoints_root || !mp)
    {
      printk (
          "vfs: remove_mountpoint: tried to remove nonexistent mountpoint\n");
      return;
    }
  if (mountpoints_root == mp)
    {
      mountpoints_root = mp->next;
      kfree (mp);
      return;
    }

  mountpoint_t *prev = mountpoints_root;
  while (prev->next && prev->next != mp)
    prev = prev->next;

  if (prev->next == mp)
    {
      prev->next = mp->next;
      kfree (mp);
    }
}

/* Mount a path */
void
vfs_mount (char *device, char *target, char *fs_type)
{
  mountpoint_t *mp = vfs_create_mountpoint (device, target, fs_type);
  if (!mp)
    {
      printk ("vfs: vfs_mount: allocation failed\n");
      return;
    }

  if (kstrcmp (fs_type, "ustar") == 0)
    {
      mp->operations = ustar_ops;
    }
  else if (kstrcmp (fs_type, "devfs") == 0)
    {
      mp->operations = devfs_ops;
    }
  else
    {
      printk ("vfs: vfs_mount: unsupported filesystem\n");
      return;
    }

  vfs_add_mountpoint (mp);
  printk ("vfs: mounted %s fs on %s\n", fs_type, target);
}

/* Unmount a path */
void
vfs_umount (char *device, char *target)
{
  mountpoint_t *cur = mountpoints_root;
  while (cur)
    {
      int dev_match = (device && kstrcmp (cur->device, device) == 0);
      int tgt_match = (target && kstrcmp (cur->mountpoint, target) == 0);

      if (dev_match || tgt_match)
        {
          vfs_remove_mountpoint (cur);
          return;
        }
      cur = cur->next;
    }
}

void *
vfs_find_node (char *path)
{
  mountpoint_t *mp = mountpoints_root;

  while (mp)
    {
      int mount_len = kstrlen (mp->mountpoint);

      int i;
      for (i = 0; i < mount_len; i++)
        {
          if (path[i] != mp->mountpoint[i])
            break;
        }

      if (i == mount_len && (path[i] == '/' || path[i] == '\0'))
        {
          char *relpath = path + mount_len;
          if (*relpath == '/')
            relpath++;

          if (mp->operations.lookup)
            return mp->operations.lookup (NULL, relpath);

          return NULL;
        }

      mp = mp->next;
    }

  return NULL;
}

int
vfs_write (char *path, void *buffer, int size)
{
  if (!path)
    {
      printk ("vfs: vfs_write: path not found\n");
      return -1;
    }
  mountpoint_t *mp = mountpoints_root;

  while (mp)
    {
      int mount_len = kstrlen (mp->mountpoint);
      int i;
      for (i = 0; i < mount_len; i++)
        {
          if (path[i] != mp->mountpoint[i])
            break;
        }

      if (i == mount_len && (path[i] == '/' || path[i] == '\0'))
        {
          char *relpath = path + mount_len;
          if (*relpath == '/')
            relpath++;

          if (mp->operations.write)
            return mp->operations.write (relpath, buffer, size);

          return -1;
        }

      mp = mp->next;
    }

  return -1;
}

int
vfs_read (char *path, void *buffer, int size)
{
  mountpoint_t *mp = mountpoints_root;

  while (mp)
    {
      int mount_len = kstrlen (mp->mountpoint);
      int i;
      for (i = 0; i < mount_len; i++)
        {
          if (path[i] != mp->mountpoint[i])
            break;
        }

      if (i == mount_len && (path[i] == '/' || path[i] == '\0'))
        {
          char *relpath = path + mount_len;
          if (*relpath == '/')
            relpath++;

          if (mp->operations.read)
            return mp->operations.read (relpath, buffer, size);

          return -1;
        }

      mp = mp->next;
    }

  return -1;
}