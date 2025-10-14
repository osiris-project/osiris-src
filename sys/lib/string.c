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
#include <sys/string.h>

void *
memcpy (void *restrict dest, const void *restrict src, int n)
{
  uint8_t *restrict pdest = (uint8_t *restrict)dest;
  const uint8_t *restrict psrc = (const uint8_t *restrict)src;

  for (int i = 0; i < n; i++)
    {
      pdest[i] = psrc[i];
    }

  return dest;
}

void *
memset (void *s, int c, int n)
{
  uint8_t *p = (uint8_t *)s;

  for (int i = 0; i < n; i++)
    {
      p[i] = (uint8_t)c;
    }

  return s;
}

void *
memmove (void *dest, const void *src, int n)
{
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest)
    {
      for (int i = 0; i < n; i++)
        {
          pdest[i] = psrc[i];
        }
    }
  else if (src < dest)
    {
      for (int i = n; i > 0; i--)
        {
          pdest[i - 1] = psrc[i - 1];
        }
    }

  return dest;
}

int
memcmp (const void *s1, const void *s2, int n)
{
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (int i = 0; i < n; i++)
    {
      if (p1[i] != p2[i])
        {
          return p1[i] < p2[i] ? -1 : 1;
        }
    }

  return 0;
}

size_t
kstrlen (const char *s)
{
  size_t i = 0;
  while (s[i])
    i++;
  return i;
}

void
kstrcpy (char *dst, const char *src)
{
  while ((*dst++ = *src++))
    ;
}