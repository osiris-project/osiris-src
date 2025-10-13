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

#include <limine.h>
#include <sys/printk.h>
#include <sys/tar/tar_parse.h>

void
read_readme ()
{
  char buf[128];
  int bytes = 0;
  void *fd_handle = tarfs_open ("rootfs/README.txt", 0);
  if (fd_handle == NULL)
    {
      return;
    }

  bytes = tarfs_read ((char *)fd_handle, buf, sizeof (buf) - 1);

  tarfs_close (fd_handle);

  if (bytes > 0)
    {
      buf[bytes] = 0;
      printk ("%s\n", buf);
    }
  else
    {
      return;
    }
}

void
kernel_init ()
{
  read_readme ();
  for (;;)
    asm volatile ("hlt");
}
