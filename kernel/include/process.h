#pragma once
#include "types.h"
#include "vmm.h"

#define KERNEL_STACK_SIZE 0xF000
#define MAX_PROCESSES 1024


typedef enum
{
    PROCESS_RUNNING,
    PROCESS_READY,
    PROCESS_BLOCKED,
    PROCESS_DEAD,
} process_state_t;

typedef struct process
{
    uint64_t pid;
    address_space_t address_space;
    vaddr_t stack_base;
    vaddr_t rsp;
    process_state_t state;
} process_t;


static uint64_t next_pid = 1;
static process_t *process_table[MAX_PROCESSES];


process_t *create_process(vaddr_t, address_space_t);
void process_destroy(process_t *);
void process_switch(process_t *, process_t *);

