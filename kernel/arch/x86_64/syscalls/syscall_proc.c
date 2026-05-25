#include "arch/x86_64/syscall.h"
#include "task.h"
#include "scheduler.h"
#include "debug.h"

void sys_yield()
{
    task_t *current_task = scheduler_get_current();
    process_t *current_process = current_task->process;
    k_log("[SYSCALL] Yield called for pid: %d", current_process->pid);
    scheduler_yield();
}

void sys_exit()
{
    task_t *current_task = scheduler_get_current();
    k_log("[SYSCALL] Exit called for pid: %d, tid: %d", current_task->process->pid, current_task->tid);

    if (task_is_main(current_task))
    {
        k_log("[SYSCALL] Exit called on main task (%d, %d)", current_task->process->pid, current_task->tid);
        sys_exit_group();
        return;
    }

    k_log("[SYS] Yeilding");
    scheduler_remove(current_task);
    scheduler_yield();
}

void sys_exit_group()
{
    task_t *current_task = scheduler_get_current();
    k_log("[SYSCALL] Exit Group called for pid: %d, tid: %d", current_task->process->pid, current_task->tid);
    process_t *current_process = current_task->process;
    for (uint8_t i = 0; i < current_process->task_count; i++) {
        scheduler_remove(current_process->tasks[i]);
    } 
    scheduler_yield();
}

uint64_t sys_getpid() {
    task_t *current_task = scheduler_get_current();
    // k_log("[SYSCALL] Get PID called");
    return current_task->process->pid;
}

uint64_t sys_gettid() {
    task_t *current_task = scheduler_get_current();
    // k_log("[SYSCALL] Get TID called %d", current_task->tid);
    return current_task->tid;
}