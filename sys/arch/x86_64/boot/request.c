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

__attribute__ ((
    used,
    section (".limine_requests"))) static volatile LIMINE_BASE_REVISION (3);

__attribute__ ((
    used,
    section (".limine_requests"))) volatile struct limine_framebuffer_request
    framebuffer_request
    = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };

__attribute__ ((
    used, section (".limine_requests"))) volatile struct limine_memmap_request
    memmap_request
    = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };

__attribute__ ((
    used,
    section (
        ".limine_requests"))) volatile struct limine_hhdm_request hhdm_request
    = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };

__attribute__ ((
    used,
    section (
        ".limine_requests"))) volatile struct limine_executable_address_request
    kernel_address_request
    = { .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST, .revision = 0 };

__attribute__ ((
    used, section (".limine_requests"))) volatile struct limine_module_request
    module_request
    = { .id = LIMINE_MODULE_REQUEST, .revision = 0 };

__attribute__ ((
    used, section (".limine_requests_"
                   "start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__ ((
    used,
    section (
        ".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;
