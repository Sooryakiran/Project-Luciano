#include "process.h"
#include "kmalloc.h"
#include "string.h"
#include "debug.h"


void process_destroy(process_t *process)
{
    k_log("[PROCESS] Destroying process with pid %d", process->pid);
    vmm_destroy(process->address_space);
    // for(uint8_t i = 0; i < process->task_count; i++) {
    //     task_destroy(process->tasks[i]);
    // }
    process_table[process->pid] = NULL;
    kfree((void *)process);
}

void process_add_task(process_t *process, task_t *task) {
    if(process->task_count >= MAX_TASKS_PER_PROCESS) {
        k_panic("[PROCESS] Unable to add more tasks for process with pid %d", process->pid);
    }
    process->tasks[process->task_count++] = task;
}

task_t *process_get_main_task(process_t *process) {
    if (process->task_count <= 0) {
        k_panic("[PROCESS] No tasks in this process");
    }
    return process->tasks[0];
}