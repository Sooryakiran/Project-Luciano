#pragma once
#include "types.h"

void syscall_init();
void syscall_dispatch(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);