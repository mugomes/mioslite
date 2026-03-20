; Copyright (C) 2026 Murilo Gomes Julio
; SPDX-License-Identifier: GPL-2.0-only

; Site: https://github.com/mugomes

section .note.GNU-stack noalloc noexec nowrite progbits

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0)

section .bss
align 16
stackBottom:
    resb 16384
stackTop:

section .text
global _start
extern kernelMain

_start:
    mov esp, stackTop
    mov edi, stackBottom
    mov ecx, 16384 / 4
    xor eax, eax
    rep stosd

    push stackBottom

    cli
    call kernelMain

.hang:
    hlt
    jmp .hang