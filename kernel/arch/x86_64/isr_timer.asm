section .text
BITS 64

global isr_stub_32

extern exception_handler
extern scheduler_tick
extern pic_eoi
extern vmm_switch
extern process_switch

; timer would need low stack depth so that i can 
; switch process easily
isr_stub_32:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    mov rdi, 32
    call pic_eoi

    ; i need to allocate 2 pointers
    sub rsp, 16
    lea rdi, [rsp]
    lea rsi, [rsp + 8]
    
    call scheduler_tick

    test al, al
    jz .no_switch

    mov rdi, [rsp]       ; read back from stack, not from rdi
    mov rsi, [rsp + 8]
    add rsp, 16

    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov [rdi + 0x18], rsp    ; current->rsp
    mov rbx, rsi
    mov rdi, [rsi + 0x08]    ; next->address_space
    call vmm_switch
    mov rsp, [rbx + 0x18]    ; next->rsp

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    jmp .done

.no_switch:
    add rsp, 16
    jmp .done

.done:
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    iretq