BITS 64

section .data
    path db "/C/foo.txt", 0   ; null terminated

section .text        
global _start
_start:
    mov rax, 2              ; sys_open
    lea rdi, [rel path]     ; arg1 = path pointer
    mov rsi, 0              ; arg2 = flags (0 = O_RDONLY)
    mov rdx, 0              ; arg3 = mode (0 if not creating)
    syscall
    jmp _start