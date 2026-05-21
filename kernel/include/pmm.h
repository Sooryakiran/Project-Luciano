#pragma once
#include "types.h"

#define PMM_FRAME_SIZE 4096


typedef enum {
    MEM_USABLE,
    MEM_RESERVED,
    MEM_KERNEL, 
    MEM_BAD,
    MEM_FRAMEBUFFER,
    MEM_ACPI,
    MEM_BOOTLOADER
} mem_region_type;

typedef struct {
    uint64_t offset;
    uint64_t size;
    mem_region_type type;
} mem_region;

void pmm_init(mem_region *regions, size_t count, uint64_t hhdm_offset);
paddr_t pmm_alloc();
void pmm_free(uint64_t addr); 
uint64_t pmm_get_total_memory();