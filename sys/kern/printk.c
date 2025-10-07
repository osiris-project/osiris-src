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

#include <stdarg.h>
#include <stddef.h>
#include <osiris/dev/liminefb.h>
#include <osiris/kern/printk.h>

int
vsnprintf (char *buffer, int size, const char *fmt, va_list args)
{
  char *buf = buffer;
  while (*fmt && (buf - buffer) < size - 1)
    {
      if (*fmt != '%')
        {
          *buf++ = *fmt++;
          continue;
        }

      fmt++;
      if (*fmt == 's')
        {
          char *s = va_arg (args, char *);
          while (*s && (buf - buffer) < size - 1)
            *buf++ = *s++;
        }
      else if (*fmt == 'd' || *fmt == 'x')
        {
          int num = va_arg (args, int);
          int base = (*fmt == 'x') ? 16 : 10;
          char tmp[11];
          int i = 0;
          while (num && i < 10)
            {
              tmp[i++] = "0123456789ABCDEF"[num % base];
              num /= base;
            }
          while (i--)
            {
              *buf++ = tmp[i];
            }
        }
      else if (*fmt == 'u'
               || (*fmt == 'l' && *(fmt + 1) == 'l' && *(fmt + 2) == 'u'))
        {
          uint64_t num;
          if (*fmt == 'u')
            {
              num = va_arg (args, unsigned int);
            }
          else
            {
              fmt += 2;
              num = va_arg (args, unsigned long long);
            }
          char tmp[21];
          int i = 0;
          if (num == 0)
            tmp[i++] = '0';
          while (num)
            {
              tmp[i++] = "0123456789"[num % 10];
              num /= 10;
            }
          while (i--)
            *buf++ = tmp[i];
        }
      else if (*fmt == 'l' && *(fmt + 1) == 'l' && *(fmt + 2) == 'x')
        {
          fmt += 2;
          uint64_t num = va_arg (args, uint64_t);
          char tmp[17];
          int i = 0;
          if (num == 0)
            tmp[i++] = '0';
          int width = 16;
          if (num == 0)
            tmp[i++] = '0';
          while (num && i < width)
            {
              tmp[i++] = "0123456789ABCDEF"[num % 16];
              num /= 16;
            }
          while (i < width)
            {
              tmp[i++] = '0';
            }
          while (i--)
            *buf++ = tmp[i];
        }
      else
        {
          *buf++ = *fmt;
        }
      fmt++;
    }
  *buf = '\0';
  return buf - buffer;
}

void
printk (const char *fmt, ...)
{
  char buffer[1000];
  va_list args;
  va_start (args, fmt);
  vsnprintf (buffer, sizeof (buffer), fmt, args);
  va_end (args);
  dev_liminefb_putstr (buffer, 0xffffff);
}

void
log (const char *subsystem, const char *msg)
{
  printk ("%s: %s", subsystem, msg);
}