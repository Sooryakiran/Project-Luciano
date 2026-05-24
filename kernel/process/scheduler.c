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
uint8_t scheduler_enabled = 0;

int scheduler_switch_task(task_t *current, task_t *next);
void scheduler_aux_spawn_daemon();

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

void scheduler_aux_spawn_daemon() {
    address_space_t curr_space = vmm_get_current_space();
    process_t* sch_proc_d = create_process((vaddr_t)&scheduler_task_cleanupd, curr_space, PRIVILEGE_KERNEL);
    for (uint8_t i = 0; i < sch_proc_d->task_count; i++){
        scheduler_add(sch_proc_d->tasks[i]);
    }
}

void scheduler_init()
{
    k_log("[SCH] Initializing scheduler");
    queue_length = 0;
    current_idx = 0;
    scheduler_enabled = 0;
    scheduler_init_sentinals();
    scheduler_aux_spawn_daemon();
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
    if (!scheduler_enabled) return 0;
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
    while (queue[next_idx] == NULL || queue[next_idx]->state == TASK_DEAD)
    {
        next_idx = (next_idx + 1) % queue_length;
        if (++checked == queue_length)
            return NULL; // all dead
    }
    current_idx = next_idx;
    // k_log("[SCH] Next process found with (%d, %d) and state %d", queue[next_idx]->process->pid, queue[next_idx]->tid, queue[next_idx]->state);
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
    if(current->state == TASK_RUNNING)
        current->state = TASK_READY;
    // k_log("[SCH] Switching from (%d, %d) to (%d, %d)", current->process->pid, current->tid, next->process->pid, next->tid);
    next->state = TASK_RUNNING;
}

uint8_t scheduler_tick(task_t **current_out, task_t **next_out)
{
    if (!scheduler_aux_check_validate_state())
        return 0;


    task_t *current = scheduler_aux_get_current_or_sential();
    scheduler_aux_start();
    task_t *next = scheduler_get_next();
    // k_log("[SCH] Scheduler BADVALUD");

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
    k_log("[SCH] Removing task with pid %d, tid %d", task->process->pid, task->tid);
    for (uint64_t i = 0; i < queue_length; i++)
    {
        if (queue[i] == task)
        {
            k_log("[SCH] Found task at index %d, marking as dead", i);
            queue[i]->state = TASK_DEAD;
            k_log("[SCH] Task (%d, %d) has state %d", queue[i]->process->pid, queue[i]->tid, queue[i]->state);
            return;
        }
    }
}

void scheduler_enable() {
    k_log("[SCH] Scheduler taking over");
    scheduler_enabled = 1;
}

void scheduler_task_cleanupd() {
    k_log("[SCH] Staring daemon task cleanup");
    while(1) {
        for (uint64_t i = 0; i < queue_length; i++) {
            if (queue[i] != NULL && queue[i]->state == TASK_DEAD) {
                k_log("[SCHD] found dead task at (%d, %d)", queue[i]->process->pid, queue[i]->tid);
                task_destroy(queue[i]);
                queue[i] = NULL;
            }
        }
    }
}
