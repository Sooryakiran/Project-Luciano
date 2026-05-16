#pragma once

#include "types.h"

typedef uint64_t gdt_entry_t;
typedef struct
{
    uint32_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flag;
} gdt;

typedef struct 
{
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdtr;


gdt_entry_t create_gdt_entry(gdt);
void gdt_init();