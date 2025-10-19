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
extern isr_handler_c
extern irq_handler_c

%macro ISR_NOERR 2
global do_isr%1
do_isr%1:
    cli
    push 0
    push %2
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 2
global do_isr%1
do_isr%1:
    cli
    push %2
    jmp isr_common_stub
%endmacro

%macro IRQ 2
global do_irq%1
do_irq%1:
    cli
    push 0
    push %2
    jmp irq_common_stub
%endmacro

isr_common_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call isr_handler_c

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

irq_common_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call irq_handler_c

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

ISR_NOERR 0, 0
ISR_NOERR 1, 1
ISR_NOERR 2, 2
ISR_NOERR 3, 3
ISR_NOERR 4, 4
ISR_NOERR 5, 5
ISR_NOERR 6, 6
ISR_NOERR 7, 7
ISR_ERR   8, 8
ISR_NOERR 9, 9
ISR_ERR   10, 10
ISR_ERR   11, 11
ISR_ERR   12, 12
ISR_ERR   13, 13 
ISR_ERR   14, 14
ISR_NOERR 15, 15
ISR_NOERR 16, 16
ISR_ERR   17, 17
ISR_NOERR 18, 18

IRQ 0, 32
IRQ 1, 33