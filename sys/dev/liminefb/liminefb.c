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
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <x86_64/request.h>
#include <liminefb.h>
#include <sys/printk.h>
#include <sys/vfs_mount.h>

/* Technically, you can use another font by just changing up the symbol names,
 * however, it must be 8x16, since some numbers are hardcoded here. It also
 * should be psf1 */
extern uint8_t _binary_lib_viscii10_8x16_psf_end;
extern uint8_t _binary_lib_viscii10_8x16_psf_size;
extern uint8_t _binary_lib_viscii10_8x16_psf_start[];

struct psf1_header
{
  uint8_t magic[2];
  uint8_t mode;
  uint8_t charsize;
};

uint32_t *fb_addr;
int pitch = 0;
int bpp = 0;
int cursor_x = 0;
int cursor_y = 0;
int cursor_prev_x = 0;
int cursor_prev_y = 0;
int fb_width = 0;
int fb_height = 0;
uint32_t bg_color = 0x00000000;

struct psf1_header *font
    = (struct psf1_header *)&_binary_lib_viscii10_8x16_psf_start;

void
liminefb_erase_cursor ()
{
  int cursor_height = 2;
  int cursor_width = 8;
  for (int y = 0; y < cursor_height; y++)
    {
      for (int x = 0; x < cursor_width; x++)
        {
          int pixel_index = (cursor_prev_y + font->charsize - cursor_height + y)
                                * (pitch / (bpp / 8))
                            + (cursor_prev_x + x);
          fb_addr[pixel_index] = bg_color;
        }
    }
}

void
liminefb_redraw_cursor ()
{
  if (cursor_x + 8 > fb_width)
    {
      liminefb_newline ();
    }
  liminefb_erase_cursor ();

  if (cursor_y > fb_height)
    {
      cursor_y = cursor_y - 16;
    }
  int cursor_height = 2;
  int cursor_width = 8;
  int row_pixels = pitch / (bpp / 8);

  for (int y = 0; y < cursor_height; y++)
    {
      int fb_y = cursor_y + font->charsize - cursor_height + y;
      if (fb_y >= fb_height)
        continue;

      for (int x = 0; x < cursor_width; x++)
        {
          int fb_x = cursor_x + x;
          if (fb_x >= fb_width)
            continue;

          fb_addr[fb_y * row_pixels + fb_x] = 0xFFFFFF;
        }
    }

  cursor_prev_x = cursor_x;
  cursor_prev_y = cursor_y;
}

void
liminefb_scroll ()
{
  liminefb_erase_cursor ();
  int char_height = 16;
  uint8_t *byte_fb = (uint8_t *)fb_addr;

  for (int y = 0; y < fb_height - char_height; y++)
    {
      for (int x = 0; x < pitch; x++)
        {
          byte_fb[y * pitch + x] = byte_fb[(y + char_height) * pitch + x];
        }
    }

  for (int y = fb_height - char_height; y < fb_height; y++)
    {
      for (int x = 0; x < pitch; x++)
        {
          byte_fb[y * pitch + x] = 0;
        }
    }

  cursor_x = 0;
  cursor_y = fb_height - 16;
  liminefb_redraw_cursor ();
}

void
liminefb_newline ()
{
  cursor_x = 0;
  cursor_y += 16;

  if (cursor_y + 16 > fb_height)
    {
      liminefb_scroll ();
    }

  liminefb_redraw_cursor ();
}

void
liminefb_erase_char ()
{
  for (int i = 0; i < 16; i++)
    {
      for (int j = 0; j < 8; j++)
        {
          int px = (cursor_y + i) * (pitch / (bpp / 8)) + (cursor_x + j);
          fb_addr[px] = 0x000000;
        }
    }
  liminefb_redraw_cursor ();
}

void
liminefb_putchar (char c, uint32_t color)
{
  if (cursor_x + 8 > fb_width)
    liminefb_newline ();
  if (cursor_y + 16 > fb_height)
    liminefb_scroll ();
  if (c == '\b')
    {
      if (cursor_x > 0)
        {
          cursor_x -= 8;
          liminefb_erase_char ();
        }
      else if (cursor_y > 0)
        {
          cursor_y -= 16;
          cursor_x = (fb_width / 8 - 1) * 8;
          liminefb_erase_char ();
        }
      return;
    }
  if (c == '\n')
    {
      liminefb_newline ();
      return;
    }
  uint8_t *glyphs
      = &_binary_lib_viscii10_8x16_psf_start[sizeof (struct psf1_header)];

  uint8_t *glyph = glyphs + (c * font->charsize);

  for (int i = 0; i < font->charsize; i++)
    {
      for (int j = 0; j < 8; j++)
        {
          if (glyph[i] & (0x80 >> j))
            {
              int baseline_offset = 2;
              int pixel_index
                  = (cursor_y + i - baseline_offset) * (pitch / (bpp / 8))
                    + (cursor_x + j);
              fb_addr[pixel_index] = color;
            }
        }
    }
  cursor_x = cursor_x + 8;
  liminefb_redraw_cursor ();
}

void
liminefb_putstr (char *str, uint32_t color)
{
  while (*str)
    {
      liminefb_putchar (*str, color);
      str++;
    }
}

void
liminefb_init ()
{
  const struct limine_framebuffer *framebuffer
      = framebuffer_request.response->framebuffers[0];
  fb_addr = framebuffer->address;
  pitch = framebuffer->pitch;
  bpp = framebuffer->bpp;
  fb_width = framebuffer->width;
  fb_height = framebuffer->height;
}

int
liminefb_write (char *node, void *buffer, int size)
{
  /* Both are unused for now*/
  (void)node;
  (void)size;
  liminefb_putstr ((char *)buffer, 0xffffff);
  return 0;
}

fs_operations_t liminefb_ops = {
  .open = NULL,
  .close = NULL,
  .read = NULL,
  .write = liminefb_write,
};