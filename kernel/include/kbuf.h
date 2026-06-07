#pragma once
#include "types.h"
#include "kmalloc.h"
#include "string.h"

typedef struct kbuf
{
    char *data;
    size_t allocated;
} kbuf_t;

k_status kbuf_grow(kbuf_t *buf);
k_status kbuf_init(kbuf_t *buf, size_t target_size);
