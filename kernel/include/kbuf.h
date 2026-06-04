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
