#pragma once

#include "types.h"

void *memset(void *ptr, int value, size_t size);
void *memcpy(void *dst, void *src, size_t size);
int strcmp(const char *a, const char *b);
int memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *a);