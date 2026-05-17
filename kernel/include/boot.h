#pragma once

#include "pmm.h"
#include "types.h"

typedef struct
{
    mem_region *regions;
    size_t region_count;
    uint64_t hhdm_offset;
} boot_info;


void get_boot_entry(boot_info *info);