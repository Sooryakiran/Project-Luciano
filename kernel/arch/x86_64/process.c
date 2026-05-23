#include "process.h"
#include "debug.h"
#include "kmalloc.h"
#include "string.h"
#include "task.h"

process_t *create_process(vaddr_t entry_point, address_space_t address_space, privilege_t privilege)
{
    k_log("[PROCESS] Creating process");
    process_t *new_process = (process_t *)kmalloc(sizeof(process_t));
    new_process->pid = next_pid++;
    new_process->task_count = 0;
    new_process->privilege = privilege;
    if (new_process->pid >= MAX_PROCESSES)
    {
        k_panic("[PROCESS] Process table full");
    }
    new_process->address_space = address_space;
    
    // hardcode kernel space for now
    task_t *task = task_create(new_process, entry_point);
    process_add_task(new_process, task);
    
    // todo add to scheduler once scheduler code is changed to use tasks

    process_table[new_process->pid] = new_process;
    k_log("[PROCESS] Process created with pid %d", new_process->pid);
    return new_process;
}