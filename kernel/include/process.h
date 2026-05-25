#pragma once
#include "types.h"
#include "vmm.h"
#include "task.h"
#include "vfs.h"

#define MAX_TASKS_PER_PROCESS 256
#define KERNEL_STACK_SIZE 0x20000   // 64KB
#define USER_STACK_SIZE 0x100000  // 1MB
#define MAX_PROCESSES 1024
#define MAX_FILE_DESCRIPTORS 1024

struct task;
struct file_descriptor;
typedef struct process
{
    uint64_t pid;
    address_space_t address_space;
    struct task *tasks[MAX_TASKS_PER_PROCESS];
    uint8_t task_count;
    privilege_t privilege;
    struct file_descriptor *fds[MAX_FILE_DESCRIPTORS];
} process_t;


static uint64_t next_pid = 1;
static process_t *process_table[MAX_PROCESSES];


process_t *create_process(vaddr_t, address_space_t, privilege_t);
void process_add_task(process_t *, struct task*);
void process_destroy(process_t *);
struct task * process_get_main_task(process_t *);

