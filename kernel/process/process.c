#include "process.h"
#include "kmalloc.h"
#include "string.h"
#include "debug.h"

static uint64_t next_pid = 1;
static process_t *process_table[MAX_PROCESSES];

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

    k_log("Entry point is %x", entry_point);
    uint8_t *stack_base = kmalloc(KERNEL_STACK_SIZE);
    new_process->stack_base = (vaddr_t)stack_base;

    k_log("stack top = %x", (uint64_t)(stack_base + KERNEL_STACK_SIZE));

    uint64_t *stack = (uint64_t *)(stack_base + KERNEL_STACK_SIZE);
    k_log("entry stored at = %x", (uint64_t)(stack - 1));
    stack--;
    *stack = entry_point;

    // fake callee saved registers
    stack -= 6;
    memset(stack, 0, 6 * sizeof(uint64_t));

    new_process->rsp = (vaddr_t)stack;
    k_log("rsp = %x", new_process->rsp);
    process_table[new_process->pid] = new_process;
    k_log("Process created with pid %d", new_process->pid);
    return new_process;
}

void process_destroy(process_t *process)
{
    k_log("[PROCESS] Destroying process with pid %d", process->pid);
    vmm_destroy(process->address_space);
    kfree((void *)(process->stack_base));
    process_table[process->pid] = NULL;
    kfree((void *)process);
}

