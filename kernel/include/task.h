#pragma once
#include "process.h"
#include "types.h"

struct process;
typedef enum
{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_DEAD,
} task_state_t;

typedef struct task
{
    vaddr_t stack_pointer;
    struct process *process;
    vaddr_t kernel_stack_base;
    vaddr_t kernel_stack_top;
    vaddr_t user_stack_base;
    vaddr_t user_stack_top;
    uint64_t tid;
    task_state_t state;
} task_t;

task_t *task_create(struct process *process, vaddr_t entry_point);
void task_destroy(task_t *task);