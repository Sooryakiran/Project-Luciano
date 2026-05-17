#include "boot.h"
#include "debug.h"
#include "limine.h"
#include "boot.h"
#include "types.h"

#define LIMINE_MEMMAP_REQUEST_ID {LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62}
// #define LIMINE_HHDM_REQUEST {LIMINE_COMMON_MAGIC, 0x48dcf1cb8ad2b852, 0x63984e959a98244b}

#define MAX_CHUNKS 256;

static volatile struct limine_memmap_request memmap_request =
    {
        .id = LIMINE_MEMMAP_REQUEST_ID,
        .revision = 0,
        .response = NULL};

static volatile struct limine_hhdm_request hhdm_request =
    {
        .id = LIMINE_HHDM_REQUEST,
        .response = NULL
    };

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

    k_log("[BOOT][LIMINE] Getting HHDM offset");
    if (hhdm_request.response == NULL) {
        k_panic("[BOOT][LIMINE] Unable to ged HHDM offset");
    }
    info->hhdm_offset = hhdm_request.response->offset;
    k_log("[BOOT][LIMINE] Obtained HHDM offset as %lu", info->hhdm_offset);
}
