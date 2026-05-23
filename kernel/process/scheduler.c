#include "scheduler.h"
#include "types.h"
#include "process.h"
#include "debug.h"
#include "kmalloc.h"

process_t* queue[MAX_PROCESSES];
process_t* sentient;

uint64_t queue_length = 0;
uint64_t current_idx = 0;
uint8_t scheduler_running = 0;

void scheduler_init() {
    k_log("[SCH] Initializing scheduler");
    queue_length = 0;
    current_idx = 0;

    // create a dummy process for current kernel context
    sentient = (process_t *)kmalloc(sizeof(process_t));
    sentient->address_space = vmm_get_current_space();
    sentient->pid = -1;
    sentient->rsp = 0;
    sentient->stack_base = 0;
    sentient->stack_base = PROCESS_READY;

    k_log("[SCG] Done!");
}

void scheduler_add(process_t *process) {
    k_log("[SCH] Adding process with pid %d", process->pid);
    queue[queue_length++] = process;
    k_log("[SCH] Current queue length is %d", queue_length);
}

void scheduler_start() {
}

uint8_t scheduler_tick(process_t **current_out, process_t **next_out) {
    // k_log("[SCH] tick: queue length is %d", queue_length);
    if (queue_length == 0) {
        k_log("[SCH] empty queue, skipping");
        return 0;
    }

    process_t *current;
    if(scheduler_running == 0) {
        current = sentient;
        scheduler_running = 1;
    } else {
        current = queue[current_idx];
    }


    current_idx = (current_idx + 1) % queue_length;
    process_t *next = queue[current_idx];
    if (current == next) return 0;
    current->state = PROCESS_READY;
    next->state = PROCESS_RUNNING;
    // k_log("[SCH] Scheduler going to switch from pid %d to %d", current->pid, next->pid);
    // process_switch(current, next);
    *current_out = current;
    *next_out = next;
    return 1;
}