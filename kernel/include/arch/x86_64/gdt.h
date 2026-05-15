#pragma once

#include "types.h"

typedef uint64_t gdt_entry_t;
typedef struct
{
    uint32_t base;
    uint32_t limit;
    uint8_t acess_byte;
    uint8_t flag;
} gdt;

gdt_entry_t create_gdt_entry(gdt);
void gdt_init();