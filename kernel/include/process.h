#pragma once
#include "types.h"
#include "vmm.h"
#include "task.h"

#define MAX_TASKS_PER_PROCESS 256
#define KERNEL_STACK_SIZE 0xF000
#define MAX_PROCESSES 1024

struct task;

typedef struct process
{
    uint64_t pid;
    address_space_t address_space;
    struct task *tasks[MAX_TASKS_PER_PROCESS];
    uint8_t task_count;
} process_t;


static uint64_t next_pid = 1;
static process_t *process_table[MAX_PROCESSES];


process_t *create_process(vaddr_t, address_space_t);
void process_add_task(process_t *, struct task*);
void process_destroy(process_t *);

