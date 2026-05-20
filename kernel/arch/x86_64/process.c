#include "process.h"
#include "debug.h"

__attribute__((naked)) void process_switch(process_t *current, process_t *next)
{
    asm volatile(
        "push %rbp\n"
        "push %rbx\n"
        "push %r12\n"
        "push %r13\n"
        "push %r14\n"
        "push %r15\n"
        "mov %rsp, 0x18(%rdi)\n" // current->rsp
        "mov %rsi, %rbx\n"       // save next in rbx
        "mov 0x8(%rsi), %rdi\n"  // next->address_space
        "call vmm_switch\n"
        "mov 0x18(%rbx), %rsp\n" // next->rsp
        "pop %r15\n"
        "pop %r14\n"
        "pop %r13\n"
        "pop %r12\n"
        "pop %rbx\n"
        "pop %rbp\n"
        "ret\n");
}