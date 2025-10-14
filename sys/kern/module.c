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

#include <x86_64/request.h>
#include <sys/tar/tar_parse.h>
#include <sys/module.h>
#include <sys/panic.h>
#include <sys/printk.h>
#include <sys/vfs_mount.h>

void
module_init ()
{
  if (module_request.response && module_request.response->module_count > 0)
    {
      printk ("module: detected %d modules\n",
              module_request.response->module_count);
    }
  else
    {
      panic ("module: No modules or initialisation ramdisk detected");
    }
  void *tar_addr = module_request.response->modules[0]->address;
  uint64_t tar_size = module_request.response->modules[0]->size;
  printk ("module: found initrd at %llx (%dB)\n", tar_addr, tar_size);

  tarfs_init (tar_addr);
  vfs_mount ("initrd0", "/", "ustar");
}