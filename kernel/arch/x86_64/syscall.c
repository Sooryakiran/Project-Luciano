#include "arch/x86_64/syscall.h"
#include "arch/x86_64/msr.h"
#include "debug.h"
#include "types.h"
#include "scheduler.h"

#define KERNEL_GS_BASE 0xC0000102

#define EFER 0xC0000080
#define STAR 0xC0000081
#define LSTAR 0xC0000082
#define SFMASK 0xC0000084

#define SYS_YIELD 24
#define SYS_EXIT 60

uint64_t user_rsp_scratch = 0;

extern void syscall_handler();

void sys_yield();
void sys_exit();
void sys_exit_group();

void syscall_init()
{
    k_log("[SYSCALL] Initializing syscalls");
    k_log("[SYSCALL] Syscall handler address is %x", &syscall_handler);
    wrmsr(KERNEL_GS_BASE, (uint64_t)&user_rsp_scratch);
    wrmsr(EFER, rmsr(EFER) | 1);
    wrmsr(STAR, 0x0018000800000000); // kernel CS=0x08, user CS=0x18
    wrmsr(LSTAR, (uint64_t)&syscall_handler);
    wrmsr(SFMASK, 0x202);
}

void syscall_dispatch(
    uint64_t syscall_number,
    uint64_t arg1,
    uint64_t arg2,
    uint64_t arg3,
    uint64_t arg4,
    uint64_t arg5)
{
    k_log("[SYSCALL] Wow syscall %lu  and arf %lu", syscall_number, arg1);
    switch (syscall_number)
    {
    case SYS_YIELD:
        sys_yield();
        break;
    case SYS_EXIT:
        sys_exit();
        break;
    default:
        break;
    }
}

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
