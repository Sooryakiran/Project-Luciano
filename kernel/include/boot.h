#pragma once

#include "pmm.h"
#include "types.h"

#define MAX_FRAMEBUFFERS 8

typedef struct {
    vaddr_t address;
    uint32_t height;
    uint32_t width;
    uint32_t pitch;
    uint32_t bpp;
} framebuffer_t;

typedef struct
{
    mem_region *regions;
    size_t region_count;
    uint64_t hhdm_offset;
    paddr_t kernel_physical_addr;
    vaddr_t kernel_virtual_addr;
    framebuffer_t framebuffer[MAX_FRAMEBUFFERS];
    uint8_t framebuffer_count;
} boot_info;


void get_boot_entry(boot_info *info);