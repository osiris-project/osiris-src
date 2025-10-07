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

#include <osiris/dev/atkbd.h>
#include <osiris/dev/atkbd_keymap.h>
#include <osiris/dev/liminefb.h>
#include <osiris/kern/portb.h>
#include <osiris/kern/printk.h>
#include <osiris/lib/strcmp.h>

char key_buffer[256];
volatile int buffer_start = 0;
volatile int buffer_end = 0;

#define SET_LEDS 0xED
#define ECHO 0xEE
#define GET_SET 0xF0
#define READ_SET 0x00
#define IDENTIFY 0xF2
#define SET_RATE 0xF3
#define ENABLE 0xF4
#define DISABLE 0xF5
#define RESEND 0xFE
#define RESET 0xFF

#define SCROLL_LOCK_LED 0x01
#define NUM_LOCK_LED 0x02
#define CAPS_LOCK_LED 0x04

#define ACK 0xFA
#define TIMEOUT 10000

bool shift_down = false;
bool ctrl_down = false;
bool alt_down = false;
bool caps_lock = false;

/* Current state of LEDS */
uint8_t current_led_mask = 0x00;

/* Add a character to the buffer*/
void
dev_atkbd_add_buffer (char ch)
{
  asm volatile ("cli");

  if ((buffer_end + 1) % 256 != buffer_start)
    {
      key_buffer[buffer_end] = ch;
      buffer_end = (buffer_end + 1) % 256;
    }

  asm volatile ("sti");
}

/* Clear the buffer */
void
dev_atkbd_clear_buffer ()
{
  asm volatile ("cli");
  for (int i = 0; i < 256; i++)
    {
      key_buffer[i] = 0;
    }
  buffer_start = 0;
  buffer_end = 0;
  asm volatile ("sti");
}

/* Wait for 8042 controller to be ready */
void
dev_atkbd_wait_for_kbc_write ()
{
  while (inb (0x64) & 0x02)
    ;
}

/* Wait for ACK */
bool
dev_atkbd_wait_for_response (uint8_t expected_response)
{
  int timeout = TIMEOUT;
  uint8_t response;

  while (timeout-- > 0)
    {
      if (inb (0x64) & 0x01)
        {
          response = inb (0x60);
          if (response == expected_response)
            {
              return true;
            }
        }
    }

  return false;
}

/* Send a command to 0x60 */
bool
dev_atkbd_send_command (uint8_t command)
{
  dev_atkbd_wait_for_kbc_write ();
  outb (0x60, command);
  return dev_atkbd_wait_for_response (ACK);
}

/* Get current scancode set. */
int
dev_atkbd_query_set ()
{
  /* If there's no reply, we'll just assume set 1. Emulators usually don't
   * bother with replying. Sadly, this assumption is very risky on real
   * hardware. We'll eventually get a way to not have to assume things like this
   */
  dev_atkbd_send_command (GET_SET);
  if (inb (0x60) != 0xFA)
    goto no_reply;

  if (!dev_atkbd_wait_for_response (ACK))
    goto no_reply;

  dev_atkbd_send_command (READ_SET);
  if (inb (0x60) != 0xFA)
    goto no_reply;

  uint8_t set = inb (0x60);

  return set;
no_reply:
  log ("atkbd", "dev_atkbd_query_set: no reply\n");
  set = 0x01;
  return set;
}

/* Set a LED
 * 1 - scroll lock
 * 2 - num lock
 * 3 - caps lock
 */
void
dev_atkbd_set_led (uint8_t led)
{
  dev_atkbd_wait_for_kbc_write ();
  outb (0x60, SET_LEDS);
  dev_atkbd_wait_for_kbc_write ();
  outb (0x60, led);
  dev_atkbd_wait_for_response (ACK);
}

void
dev_atkbd_disable ()
{
  outb (0x60, DISABLE);
}

/* Don't do this much, this is known to cause issues */
void
dev_atkbd_enable ()
{
  outb (0x60, ENABLE);
}

/* Decode a normal scancode */
char
dev_atkbd_decode (uint8_t scancode)
{
  char decoded_scancode;
  if (shift_down == true || caps_lock == true)
    {
      decoded_scancode = higher_kbdus[scancode];
    }
  else
    {
      decoded_scancode = lower_kbdus[scancode];
    }
  if (ctrl_down)
    {
      if (decoded_scancode >= 'a' && decoded_scancode <= 'z')
        {
          return decoded_scancode - 'a' + 1;
        }
      else if (decoded_scancode >= 'A' && decoded_scancode <= 'Z')
        {
          return decoded_scancode - 'A' + 1;
        }
    }
  return decoded_scancode;
}

/* Decode a extended scancode. This is used for buttons like TAB, SHIFT, etc */
char *
dev_atkbd_decode_extended (uint8_t scancode)
{
  char *decoded_scancode;
  decoded_scancode = keynames[scancode];
  return decoded_scancode;
}

/* Process an extended scancode. */
void
dev_atkbd_process_extended (uint8_t scancode)
{
  switch (scancode)
    {
    case 0x2A:
      shift_down = true;
      break;
    case 0x36:
      shift_down = true;
      break;
    case 0x1D:
      ctrl_down = true;
      break;
    case 0x38:
      alt_down = true;
      break;
    case 0x1C:
      dev_atkbd_add_buffer ('\n');
      break;
    case 0x0E:
      dev_atkbd_add_buffer ('\b');
      break;
    default:
      return;
    }
}

/* Check if a scancode is break */
bool
dev_atkbd_scancode_is_break (uint8_t scancode)
{
  if (scancode > 128)
    {
      return true;
    }
  else
    {
      return false;
    }
}

/* If caps_lock is true, set it to false, if it's false, set it to true. Also
 * updates keyboard LEDs*/
void
dev_atkbd_handle_capslock ()
{
  if (caps_lock == true)
    {
      caps_lock = false;
      current_led_mask &= ~CAPS_LOCK_LED;
    }
  else
    {
      caps_lock = true;
      current_led_mask |= CAPS_LOCK_LED;
    }
  dev_atkbd_set_led (current_led_mask);
}

/* This might look like dev_atkbd_process_extended() using if but I assure you
 * it's not. This is supposed to be used on break keys*/
void
dev_atkbd_handle_break (uint8_t scancode)
{
  switch (scancode)
    {
    case 0x2A:
    case 0x36:
      shift_down = false;
      break;
    case 0x1D:
      ctrl_down = false;
      break;
    case 0x38:
      alt_down = false;
      break;
    default:
      return;
    }
}

/* Main IRQ */
void
dev_atkbd_irq ()
{
  uint8_t scancode = inb (0x60);
  if (scancode == 0x3A)
    dev_atkbd_handle_capslock ();
  if (dev_atkbd_scancode_is_break (scancode))
    {
      uint8_t make_code = scancode & 0x7F;
      dev_atkbd_handle_break (make_code);
    }
  else
    {
      dev_atkbd_process_extended (scancode);
      char ch = dev_atkbd_decode (scancode);
      if (ch != 0)
        {
          dev_atkbd_add_buffer (ch);
          dev_liminefb_putchar (ch, 0xffffff);
        }
    }
  outb (0x20, 0x20);
}

void
dev_atkbd_init ()
{
  dev_atkbd_clear_buffer ();
  /* This seems to fix Qemu's keyboard not working sometimes */
  dev_atkbd_enable ();
  int set = dev_atkbd_query_set ();
  printk ("atkbd: scancode set=%d\n", set);

  /* Make sure there's nothing wrong with the LEDS */
  current_led_mask = 0x00;
  dev_atkbd_set_led (current_led_mask);
}

/* Blocking function to retrieve the next typed char */
char
dev_atkbd_get_char ()
{
  while (buffer_start == buffer_end)
    ;

  asm volatile ("cli");
  char ch = key_buffer[buffer_start];
  buffer_start = (buffer_start + 1) % 256;
  asm volatile ("sti");

  return ch;
}