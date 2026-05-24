#include "scheduler.h"
#include "types.h"
#include "process.h"
#include "task.h"
#include "debug.h"
#include "kmalloc.h"
#include "arch/x86_64/tss.h"

task_t *queue[MAX_PROCESSES];
task_t *sentinal_task;
process_t *sentinal_process;

uint64_t queue_length = 0;
uint64_t current_idx = 0;
uint8_t scheduler_running = 0;

int scheduler_switch_task(task_t *current, task_t *next);

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

uint8_t scheduler_aux_check_validate_state()
{
    // k_log("[SCH] tick: queue length is %d", queue_length);
    if (queue_length == 0)
    {
        // k_log("[SCH] empty queue, skipping");
        return 0;
    }
    return 1;
}

task_t *scheduler_aux_get_current_or_sential()
{
    return scheduler_running ? queue[current_idx] : sentinal_task;
}

task_t *scheduler_get_next()
{
    uint64_t next_idx = (current_idx + 1) % queue_length;
    uint64_t checked = 0;
    while (queue[next_idx] == NULL   || queue[next_idx]->state == TASK_DEAD)
    {
        next_idx = (next_idx + 1) % queue_length;
        if (++checked == queue_length)
            return NULL; // all dead
    }
    current_idx = next_idx;
    return queue[next_idx];
}

void scheduler_aux_start()
{
    scheduler_running = 1;
}

uint8_t scheduler_aux_validate_switch(task_t *current, task_t *next)
{
    // todo add more validations
    return current != next;
}

void scheduler_aux_update_state(task_t *current, task_t *next)
{
    current->state = TASK_READY;
    next->state = TASK_RUNNING;
    k_log("[SCH] Scheduler going to switch from (pid, tid) (%d, %d) to (%d, %d)", current->process->pid, current->tid, next->process->pid, next->tid);
}

uint8_t scheduler_tick(task_t **current_out, task_t **next_out)
{
    if (!scheduler_aux_check_validate_state())
        return 0;

    task_t *current = scheduler_aux_get_current_or_sential();
    scheduler_aux_start();
    task_t *next = scheduler_get_next();

    if (!scheduler_aux_validate_switch(current, next))
        return 0;
    scheduler_aux_update_state(current, next);
    *current_out = current;
    *next_out = next;
    return 1;
}

task_t *scheduler_get_current()
{
    return queue[current_idx];
}

void scheduler_yield()
{
    k_log("[SCH] Gonna yeild using software interrupt.");
    asm volatile("int $0x30");
}

void scheduler_remove(task_t *task)
{
    // O(n), i will use a more complicated scheduler later
    // i will implement all interfaces and make sure the system is working

    for (uint64_t i = 0; i < queue_length; i++)
    {
        if (queue[i] == task)
        {
            queue[i]->state = TASK_DEAD;
            return;
        }
    }
}