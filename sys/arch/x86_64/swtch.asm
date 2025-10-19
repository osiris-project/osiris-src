;
; SPDX-License-Identifier: 0BSD
;
; Copyright (c) 2025 V. Prokopenko
;
; Permission to use, copy, modify, and/or distribute this software for any
; purpose with or without fee is hereby granted.
;
; THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
; WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
; ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
; OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
; CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;
global proc_switch_x64
global switch_to_user
extern current_proc

USER_STACK_TOP equ 0x400000

section .text
proc_switch_x64:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov [rdi], rsp
    mov rsp, rsi

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    sti

    ret

switch_to_user:
    mov ax, 0x33
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x33
    push USER_STACK_TOP
    pushfq
    pop rax
    or rax, 0x200
    push rax
    push 0x2B
    ; push [placeholder for userspace elf binaries later]
    iretq
