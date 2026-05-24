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
#define SYS_EXIT_GROUP 231
#define SYS_GETPID 39
#define SYS_GETTID 186

uint64_t user_rsp_scratch = 0;

#ifndef UNIT_TEST
extern void syscall_handler();
#else
void syscall_handler() {}
#endif

void sys_yield();
void sys_exit();
void sys_exit_group();
uint64_t sys_getpid();
uint64_t sys_gettid();

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

uint64_t syscall_dispatch(
    uint64_t syscall_number,
    uint64_t arg1,
    uint64_t arg2,
    uint64_t arg3,
    uint64_t arg4,
    uint64_t arg5)
{
    k_log("[SYSCALL] Wow syscall %lu and arf %lu", syscall_number, arg1);
    switch (syscall_number)
    {
    case SYS_YIELD:
        sys_yield();
        return 0;
    case SYS_EXIT:
        sys_exit();
        return 0;
    case SYS_EXIT_GROUP:
        sys_exit_group();
        return 0;
    case SYS_GETPID:
        return sys_getpid();
    case SYS_GETTID:
        return sys_gettid();
    default:
        return 0;
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

uint64_t sys_getpid() {
    task_t *current_task = scheduler_get_current();
    k_log("[SYSCALL] Get PID called");
    return current_task->process->pid;
}

uint64_t sys_gettid() {
    task_t *current_task = scheduler_get_current();
    k_log("[SYSCALL] Get TID called %d", current_task->tid);
    return current_task->tid;
}