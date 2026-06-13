#pragma once
#include "types.h"

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

#define SYS_OPEN 2


void syscall_init();
uint64_t syscall_dispatch(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

// proc
void sys_yield();
void sys_exit();
void sys_exit_group();
uint64_t sys_getpid();
uint64_t sys_gettid();

// vfs
uint64_t sys_vfs_open(uint64_t arg1, uint64_t arg2, uint64_t arg3);