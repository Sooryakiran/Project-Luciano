#include "arch/x86_64/syscall.h"
#include "arch/x86_64/msr.h"
#include "debug.h"
#include "types.h"

#define KERNEL_GS_BASE 0xC0000102

#define EFER 0xC0000080
#define STAR 0xC0000081
#define LSTAR 0xC0000082
#define SFMASK 0xC0000084

uint64_t user_rsp_scratch = 0;

extern void syscall_handler();

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

void syscall_dispatch(uint64_t syscall_number)
{
    k_log("[SYSCALL] Wow syscall %lu", syscall_number);
}