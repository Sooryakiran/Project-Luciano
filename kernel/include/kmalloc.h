#pragma once
#include "types.h"

void kmalloc_init();
void *kmalloc(size_t size);
void kfree(void* ptr);