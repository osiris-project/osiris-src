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
#include <sys/devfs/devfs_dev.h>
#include <sys/panic.h>
#include <sys/printk.h>
#include <sys/strcmp.h>
#include <sys/string.h>
#include <sys/vfs_mount.h>
#include <x86_64/heap.h>

devfs_node_t *devfs_root = NULL;

/* Read from a node */
int
devfs_read (char *path, void *buffer, int size)
{
  devfs_node_t *dev = devfs_lookup (NULL, path);
  if (dev->ops && dev->ops->read)
    return dev->ops->read (dev->data, buffer, size);
  return -1;
}

/* Write to a node */
int
devfs_write (char *node, void *buffer, int size)
{
  devfs_node_t *dev = devfs_lookup (NULL, node);
  if (!dev)
    {
      printk ("devfs: devfs_write: %s not found", node);
      return -1;
    }
  if (dev->ops && dev->ops->write)
    return dev->ops->write (dev->data, buffer, size);
  return -1;
}

/* Register a device */
void
devfs_register (char *name, fs_operations_t *ops, void *data)
{
  devfs_node_t *node = kmalloc (sizeof (devfs_node_t));
  if (!node)
    {
      /* This is probably worth a panic */
      panic ("devfs: failed to allocate node");
    }
  kstrcpy (node->name, name);
  node->ops = ops;
  node->data = data;
  node->next = devfs_root;
  devfs_root = node;

  printk ("devfs: registered %s\n", name);
}

void *
devfs_lookup (void *fs_data, const char *name)
{
  (void)fs_data; /* unused */
  devfs_node_t *cur = devfs_root;
  while (cur)
    {
      if (kstrcmp (cur->name, name) == 0)
        return cur;
      cur = cur->next;
    }
  return NULL;
}

fs_operations_t devfs_ops = {
  .lookup = devfs_lookup,
  .read = devfs_read,
  .write = devfs_write,
};

/* Initialise devices.
 *  This is going to register (most) of the devices we want and need, including
 * the keyboard and console
 */
void
devfs_init ()
{
  extern char key_buffer[];
  extern fs_operations_t kbd_ops;
  extern fs_operations_t liminefb_ops;
  extern fs_operations_t random_ops;
  vfs_mount ("devfs", "/dev", "devfs");
  devfs_register ("kbd", &kbd_ops, key_buffer);

  /* dev/console is not a tty and should not be used like it is one. Instead, it
   * is what Osiris (and other unix likes!) consider the primary mean to
   * interact with the system administrator. The Linux kernel's documentation on
   * devices (https://www.kernel.org/doc/html/latest/admin-guide/devices.html)
   * says:
   *
   *
   * The console device, /dev/console, is the device to which system messages
   * should be sent, and on which logins should be permitted in single-user
   * mode. Starting with Linux 2.1.71, /dev/console is managed by the kernel;
   * for previous versions it should be a symbolic link to either /dev/tty0, a
   * specific virtual console such as /dev/tty1, or to a serial port primary
   * (tty*, not cu*) device, depending on the configuration of the system.
   *
   * Since a real TTY interface with multiple TTYs is TODO, /dev/console is the
   * primary way to communicate with the user. Only write() can be performed
   * upon /dev/console though.
   */
  devfs_register ("console", &liminefb_ops, NULL);
  devfs_register ("random", &random_ops, NULL);
}
