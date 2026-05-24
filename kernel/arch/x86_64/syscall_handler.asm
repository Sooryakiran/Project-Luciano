section .data
user_rsp_scratch: dq 0

section .text
BITS 64

global syscall_handler

extern tss
extern syscall_dispatch
extern k_log_hex


syscall_handler:
    ; swapgs
    ; mov [gs:0], rsp 
    mov [user_rsp_scratch], rsp
    mov rsp, [tss + 4] ; tss->rsp0

    ; push caller saved regs
    push rax
    push rdi
    push rsi
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11


    ; call dispatcher
    mov r8, r9
    mov r9, r10
    mov r10, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax ; rax is supposed to have syscall number
    call syscall_dispatch
    ; do anything no problem

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rsi
    pop rdi
    pop rax
    
    mov rsp, [user_rsp_scratch] ; restore
    ; mov rsp, [gs:0] 
    ; swapgs
    o64 sysret    

