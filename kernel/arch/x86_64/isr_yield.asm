section .text
BITS 64

global isr_stub_48

extern exception_handler
extern scheduler_tick
extern pic_eoi
extern vmm_switch
extern process_switch
extern tss_update
extern k_log_hex


isr_stub_48:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

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

    ; rdi is current_task rsi is next_task
    mov [rdi], rsp    ; current->rsp
    mov rbx, rsi               
    mov rbp, [rsi + 0x08]    ; rbp is next->process
    mov rdi, [rbp + 0x08]
    call vmm_switch
    mov rsp, [rbx + 0x0]    ; next->rsp
    
    ; update tss
    mov rdi, [rbx + 0x18]    ; next->kernel_stack_top
    call tss_update

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