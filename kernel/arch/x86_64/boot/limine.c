#include "boot.h"
#include "debug.h"
#include "limine.h"
#include "boot.h"
#include "types.h"

#define LIMINE_MEMMAP_REQUEST_ID {LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62}
#define MAX_CHUNKS 256;

static volatile struct limine_memmap_request memmap_request =
    {
        .id = LIMINE_MEMMAP_REQUEST_ID,
        .revision = 0,
        .response = NULL};

mem_region_type get_type(uint64_t limine_type)
{
    switch (limine_type)
    {
    case LIMINE_MEMMAP_USABLE:
        return MEM_USABLE;
    case LIMINE_MEMMAP_KERNEL_AND_MODULES:
        return MEM_KERNEL;
    default:
        return MEM_RESERVED;
    }
}

static mem_region mem_regions[256];

void get_boot_entry(boot_info *info)
{
    k_log("Getting boot info for x86_64 limine bootloader");
    if (memmap_request.response == NULL)
    {
        k_panic("No memory map from bootloader");
        return;
    }
    info->region_count = memmap_request.response->entry_count;
    info->regions = mem_regions;
    for (size_t i = 0; i < info->region_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];
        mem_regions[i] = (mem_region){
            .offset = entry->base,
            .size = entry->length,
            .type = get_type(entry->type)};
    }

    k_log("entry count is %d", info->region_count);
}
