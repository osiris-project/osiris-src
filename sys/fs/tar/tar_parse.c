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
#include <osiris/fs/tar/tar_octal.h>
#include <osiris/kern/printk.h>
#include <osiris/kern/vfs_mount.h>
#include <osiris/lib/strcmp.h>
#include <osiris/lib/string.h>

#define TAR_BLOCK_SIZE 512

typedef struct
{
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
} __attribute__ ((packed)) tar_header_t;

#define MAX_TAR_FILES 64

typedef enum
{
  TAR_FILE,
  TAR_DIR
} tar_filetype_t;

typedef struct
{
  char name[100];
  void *addr;
  uint64_t size;
  tar_filetype_t type;
} tar_file_t;

tar_file_t tar_files[MAX_TAR_FILES];
int tar_file_count = 0;

void
tarfs_init (void *tar_start)
{
  uint8_t *ptr = (uint8_t *)tar_start;
  tar_file_count = 0;

  while (1)
    {
      tar_header_t *hdr = (tar_header_t *)ptr;
      if (hdr->name[0] == '\0')
        break;
      uint64_t filesize = oct2bin (hdr->size, sizeof (hdr->size));

      if (tar_file_count < MAX_TAR_FILES)
        {
          tar_file_t *tf = &tar_files[tar_file_count++];
          size_t i = 0;
          while (i < 99 && hdr->name[i])
            {
              if (hdr->typeflag == '5')
                {
                  tf->type = TAR_DIR;
                }
              else
                {
                  tf->type = TAR_FILE;
                }
              tf->name[i] = hdr->name[i];
              i++;
            }
          tf->name[i] = 0;
          tf->addr = ptr + TAR_BLOCK_SIZE;
          tf->size = filesize;
        }

      uint64_t total = TAR_BLOCK_SIZE + ((filesize + 511) & ~511ULL);
      ptr += total;
    }
}

/* Find a file. Tar uses relative names: therefore 1.txt located in root won't
 * be 1.txt but ./1.txt, take this into account. */
void *
tarfs_find (const char *name, uint64_t *size_out)
{
  for (int i = 0; i < tar_file_count; i++)
    {
      if (kstrcmp (name, tar_files[i].name) == 0)
        {
          if (size_out)
            *size_out = tar_files[i].size;
          return tar_files[i].addr;
        }
    }
  return NULL;
}

/* Read a file */
int
tarfs_read (char *name, void *buf, int bufsize)
{
  uint64_t size;
  void *addr = tarfs_find (name, &size);
  if (!addr)
    {
      printk ("tarfs: invalid addr\n");
      return 0;
    }
  if (bufsize > size)
    bufsize = size;
  memcpy (buf, addr, bufsize);
  return bufsize;
}

fs_operations_t ustar_ops = {
  .open = NULL,
  .close = NULL,
  .read = tarfs_read,
};