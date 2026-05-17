#pragma once
#include "types.h"

typedef enum {
    MEM_USABLE,
    MEM_RESERVED,
    MEM_KERNEL
} mem_region_type;

typedef struct {
    uint64_t offset;
    uint64_t size;
    mem_region_type type;
} mem_region;

void pmm_init(mem_region *regions, size_t count, uint64_t hhdm_offset);
paddr_t pmm_alloc();
void pmm_free(uint64_t addr); 