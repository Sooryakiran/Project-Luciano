#pragma once
#include "process.h"

typedef struct task
{
    vaddr_t stack_pointer;
    process_t *process;
    vaddr_t kernel_stack_base;
    vaddr_t kernel_stack_top;
    uint64_t tid;
} task_t;

task_t *task_create(process_t *process, vaddr_t entry_point, uint8_t is_user);
void task_destroy(task_t *task);