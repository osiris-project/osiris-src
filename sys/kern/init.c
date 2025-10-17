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
#include <random.h>
#include <sys/devfs/devfs_dev.h>
#include <sys/module.h>
#include <sys/printk.h>
#include <sys/tar/tar_parse.h>
#include <sys/vfs_mount.h>

const char *copyright="Copyright (c) 2025 V. Prokopenko\nCopyright (c) 2025 The Osiris Contributors\n";

void
mi_startup ()
{
  /* Initialise modules and mount ramdisk */
  module_init ();

  /* Initialise devfs*/
  devfs_init ();

  /* Initialise random subsystem */
  random_init ();

  /* Write copyright and banner to /dev/console */
  printk("Osiris 0.00 by V. Prokopenko %s %s\n", __DATE__, __TIME__);
  vfs_write("/dev/console", copyright, sizeof (copyright));

  for (;;)
    asm volatile ("hlt");
}
