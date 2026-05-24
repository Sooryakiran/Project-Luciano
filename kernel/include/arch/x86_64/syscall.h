#pragma once
#include "types.h"

void syscall_init();
uint64_t syscall_dispatch(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);