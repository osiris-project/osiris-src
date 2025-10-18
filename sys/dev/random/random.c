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

#include <random.h>
#include <sys/mount.h>
#include <sys/printk.h>
#include <stdint.h>
#include <stddef.h>

uint64_t rand_seed;

uint64_t
random_xorshift64star (void)
{
  uint64_t x = rand_seed;
  x ^= x >> 12;
  x ^= x << 25;
  x ^= x >> 27;
  rand_seed = x;
  return x * 2685821657736338717ULL;
}

int
random_read (char *data, void *buffer, int size)
{
  (void)data; /* unused */
  uint8_t *buf = buffer;

  for (int i = 0; i < size;)
    {
      uint64_t r = random_xorshift64star ();
      for (int j = 0; j < 8 && i < size; j++)
        buf[i++] = (r >> (j * 8)) & 0xFF;
    }

  return size;
}

void
random_init ()
{
  extern int ticks;
  rand_seed = (ticks ^ 0x9E3779B97F4A7C15ULL) * 0xBF58476D1CE4E5B9ULL;
  rand_seed ^= rand_seed >> 21;
  if (rand_seed == 0)
    rand_seed = 0x9E3779B95A4A7C15ULL;
}

fs_operations_t random_ops = {
  .read = random_read,
};
