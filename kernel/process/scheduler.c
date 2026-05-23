#include "scheduler.h"
#include "types.h"
#include "process.h"
#include "task.h"
#include "debug.h"
#include "kmalloc.h"

task_t *queue[MAX_PROCESSES];
task_t *sentinal_task;
process_t *sentinal_process;

uint64_t queue_length = 0;
uint64_t current_idx = 0;
uint8_t scheduler_running = 0;

void scheduler_init_sentinals()
{
    sentinal_process = (process_t *)kmalloc(sizeof(process_t));
    sentinal_process->pid = -1;
    sentinal_process->task_count = 0;
    sentinal_process->address_space = vmm_get_current_space();

    sentinal_task = (task_t *)kmalloc(sizeof(task_t));
    sentinal_task->tid = -1;
    sentinal_task->process = sentinal_process;
    sentinal_task->stack_pointer = 0;
    sentinal_task->kernel_stack_base = 0;
    sentinal_task->kernel_stack_top = 0;
}

void scheduler_init()
{
    k_log("[SCH] Initializing scheduler");
    queue_length = 0;
    current_idx = 0;
    scheduler_init_sentinals();
    k_log("[SCG] Done!");
}

void scheduler_add(task_t *task)
{
    k_log("[SCH] Adding task with pid %d, tid %d", task->process->pid, task->tid);
    queue[queue_length++] = task;
    k_log("[SCH] Current queue length is %d", queue_length);
}

uint8_t scheduler_tick(task_t **current_out, task_t **next_out)
{
    // k_log("[SCH] tick: queue length is %d", queue_length);
    if (queue_length == 0)
    {
        // k_log("[SCH] empty queue, skipping");
        return 0;
    }

    task_t *current;
    if (scheduler_running == 0)
    {
        current = sentinal_task;
        scheduler_running = 1;
    }
    else
    {
        current = queue[current_idx];
    }

    // if (current->process->privilege == PRIVILEGE_USER)
    // {
    //     k_log("[SCH] running user task tid=%d", current->tid);
    // }
    
    current_idx = (current_idx + 1) % queue_length;
    task_t *next = queue[current_idx];
    if (current == next)
        return 0;
    current->state = TASK_READY;
    next->state = TASK_RUNNING;
    k_log("[SCH] Scheduler going to switch from (pid, tid) (%d, %d) to (%d, %d)", current->process->pid, current->tid, next->process->pid, next->tid);
    // process_switch(current, next);
    *current_out = current;
    *next_out = next;
    return 1;
}