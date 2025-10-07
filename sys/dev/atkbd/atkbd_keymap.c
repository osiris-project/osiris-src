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
#include <osiris/dev/atkbd.h>

unsigned char lower_kbdus[128]
    = { 0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
        '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
        'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
        'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' ', 0 };

unsigned char higher_kbdus[128]
    = { 0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
        '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
        'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
        'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' ', 0 };

const char *keynames[128]
    = { "ERROR", "ESC",  "1",      "2",  "3",     "4",     "5",         "6",
        "7",     "8",    "9",      "0",  "-",     "=",     "BACKSPACE", "TAB",
        "Q",     "W",    "E",      "R",  "T",     "Y",     "U",         "I",
        "O",     "P",    "[",      "]",  "ENTER", "LCtrl", "A",         "S",
        "D",     "F",    "G",      "H",  "J",     "K",     "L",         ";",
        "'",     "`",    "LShift", "\\", "Z",     "X",     "C",         "V",
        "B",     "N",    "M",      ",",  ".",     "/",     "RShift",    "*",
        "LAlt",  "SPACE" };