#include "task.h"
#include "process.h"
#include "types.h"
#include "debug.h"
#include "kmalloc.h"
#include "string.h"

#define USER_STACK_TOP(tid) ((0x00007FFF8BADBABE & ~0xFFF) - (tid) * USER_STACK_SIZE)
#define USER_STACK_BASE(tid) (USER_STACK_TOP(tid) - USER_STACK_SIZE)
#define USER_ACCESSIBLE_FLAG 0x7

uint64_t next_tid = 1;

void map_user_stack(address_space_t addr_space, uint8_t tid) {
    size_t num_frames = (USER_STACK_SIZE + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;
    for (size_t i = 0; i < num_frames; i++) {
        paddr_t physical_addr = pmm_alloc();
        vaddr_t virtual_addr = USER_STACK_BASE(tid) + i * PMM_FRAME_SIZE;
        vmm_map(addr_space, virtual_addr, physical_addr, USER_ACCESSIBLE_FLAG);
    }
}

task_t *task_create(process_t *process, vaddr_t entry_point)
{
    k_log("[TASK] Creating task in process pid: %d", process->pid);
    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    task->tid = next_tid++;
    task->process = process;

    uint8_t *kernel_stack_base = kmalloc(KERNEL_STACK_SIZE);
    task->kernel_stack_base = (vaddr_t)kernel_stack_base;
    task->kernel_stack_top = (vaddr_t)(kernel_stack_base + KERNEL_STACK_SIZE);

    if (process->privilege == PRIVILEGE_USER)
    {
        map_user_stack(process->address_space, task->tid);
        task->user_stack_base = USER_STACK_BASE(task->tid);
        task->user_stack_top = USER_STACK_TOP(task->tid);
    } else {
        task->user_stack_base = 0;
        task->user_stack_top = 0;
    }

    uint64_t *stack = (uint64_t *)(task->kernel_stack_top);
    *--stack = (process->privilege == PRIVILEGE_USER) ? 0x23 : 0x10;                                                           // SS flag
    *--stack = (process->privilege == PRIVILEGE_USER) ? (uint64_t)(task->user_stack_top) : (uint64_t)(task->kernel_stack_top); // rsp
    *--stack = 0x202;                                                                                                          // RFLAGS
    *--stack = (process->privilege == PRIVILEGE_USER) ? 0x1b : 0x08;                                                           // CS
    *--stack = entry_point;                                                                                                    // RIP

    // fake 9 scratch regs (isr stub pops these)
    stack -= 9;
    memset(stack, 0, 9 * sizeof(uint64_t));

    // fake callee saved registers
    stack -= 6;
    memset(stack, 0, 6 * sizeof(uint64_t));

    task->stack_pointer = (vaddr_t)stack;
    task->state = TASK_READY;

    return task;
}

void task_destroy(task_t *task)
{
    k_log("[TASK] I am death, destroyer or tasks");
    kfree((void *)task->kernel_stack_base);
    kfree((void *)task);
}