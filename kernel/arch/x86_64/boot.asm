; boot.asm

global _start
extern kmain

section .text

_start:
    lea rsp, k_stack_top    ; Set up the stack
    cld                     ; guarentee df is 0
    call kmain              ; call the kernel main function

.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16
k_stack_bottom:
    resb 16384 ; 16 KiB stack
k_stack_top: