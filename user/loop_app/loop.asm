BITS 64
_start:
    mov rbx, 1
    add rbx, rbx
    mov rax, 186
    syscall
    jmp _start