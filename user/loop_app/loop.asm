BITS 64
_start:
    mov rbx, 1
    add rbx, rbx
    mov rax, 60
    syscall
    jmp _start