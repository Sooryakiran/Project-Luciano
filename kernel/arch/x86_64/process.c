#include "process.h"
#include "debug.h"
#include "kmalloc.h"
#include "string.h"


process_t *create_process(vaddr_t entry_point, address_space_t address_space)
{
    k_log("[PROCESS] Creating process");
    process_t *new_process = (process_t *)kmalloc(sizeof(process_t));
    new_process->pid = next_pid++;
    if (new_process->pid >= MAX_PROCESSES)
    {
        k_panic("[PROCESS] Process table full");
    }
    new_process->address_space = address_space;
    new_process->state = PROCESS_READY;

    uint8_t *stack_base = kmalloc(KERNEL_STACK_SIZE);
    new_process->stack_base = (vaddr_t)stack_base;

    uint64_t *stack = (uint64_t *)(stack_base + KERNEL_STACK_SIZE);
    
    *--stack = 0x10;
    *--stack = (uint64_t)(stack_base + KERNEL_STACK_SIZE); // rsp
    *--stack = 0x202;                                      // RFLAGS
    *--stack = 0x08;   // CS 
    *--stack = entry_point;                                // RIP

    // fake 9 scratch regs (isr stub pops these)
    stack -= 9;
    memset(stack, 0, 9 * sizeof(uint64_t));

    // fake callee saved registers
    stack -= 6;
    memset(stack, 0, 6 * sizeof(uint64_t));

    new_process->rsp = (vaddr_t)stack;
    process_table[new_process->pid] = new_process;
    k_log("[PROCESS] Process created with pid %d", new_process->pid);
    return new_process;
}

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
        "ret\n"
    );
}