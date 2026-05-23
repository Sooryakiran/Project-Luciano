#include "task.h"
#include "process.h"
#include "types.h"
#include "debug.h"

uint64_t next_tid = 1;

task_t *task_create(process_t *process, vaddr_t entry_point, uint8_t is_user)
{
    // is_user path not implemented yet
    if (is_user)
    {
        k_log("[TASK] userspace not implemented");
        return NULL;
    }

    k_log("[TASK] Creating task");
    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    task->tid = next_tid++;
    task->process = process;

    uint8_t *stack_base = kmalloc(KERNEL_STACK_SIZE);
    task->kernel_stack_base = (vaddr_t)stack_base;
    task->kernel_stack_top = (vaddr_t)(stack_base + KERNEL_STACK_SIZE);

    uint64_t *stack = (uint64_t *)(stack_base + KERNEL_STACK_SIZE);
    *--stack = 0x10;
    *--stack = (uint64_t)(stack_base + KERNEL_STACK_SIZE); // rsp
    *--stack = 0x202;                                      // RFLAGS
    *--stack = 0x08;                                       // CS
    *--stack = entry_point;                                // RIP

    // fake 9 scratch regs (isr stub pops these)
    stack -= 9;
    memset(stack, 0, 9 * sizeof(uint64_t));

    // fake callee saved registers
    stack -= 6;
    memset(stack, 0, 6 * sizeof(uint64_t));

    task->stack_pointer = (vaddr_t)stack;
    return task;
}

void task_destroy(task_t *task) {
    k_log("[TASK] I am death, destroyer or tasks");
    kfree((void *)task->kernel_stack_base);
    kfree((void *) task);
}