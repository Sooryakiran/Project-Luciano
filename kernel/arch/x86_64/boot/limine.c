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

static volatile struct limine_hhdm_request hhdm_request =
    {
        .id = LIMINE_HHDM_REQUEST,
        .response = NULL};

static volatile struct limine_kernel_address_request k_address_request =
    {
        .id = LIMINE_KERNEL_ADDRESS_REQUEST,
        .revision = 0,
        .response = NULL};

static volatile struct limine_framebuffer_request fb_request =
    {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .revision = 0,
        .response = NULL};

mem_region_type
get_type(uint64_t limine_type)
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
    k_log("[BOOT][LIMINE] Getting boot info for x86_64 limine bootloader");
    if (memmap_request.response == NULL)
    {
        k_panic("[BOOT][LIMINE] No memory map from bootloader");
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

    k_log("[BOOT][LIMINE] entry count is %d", info->region_count);

    k_log("[BOOT][LIMINE] Getting HHDM offset");
    if (hhdm_request.response == NULL)
    {
        k_panic("[BOOT][LIMINE] Unable to ged HHDM offset");
    }
    info->hhdm_offset = hhdm_request.response->offset;
    k_log("[BOOT][LIMINE] Obtained HHDM offset as %lu", info->hhdm_offset);

    k_log("[BOOT][LIMINE] Getting kernel address...");
    if (k_address_request.response == NULL)
    {
        k_panic("[BOOT][LIMINE] Unable to get kernel addr");
    }
    info->kernel_physical_addr = k_address_request.response->physical_base;
    info->kernel_virtual_addr = k_address_request.response->virtual_base;
    k_log("[BOOT][LIMINE] Obtained kernel addresses.");

    k_log("[BOOT][LIMINE] Fetching framebuffer info");
    if (fb_request.response == NULL || fb_request.response->framebuffer_count < 1)
    {
        info->framebuffer_count = 0;
        k_log("[BOOT][LIMINE] Framebuffers not available");
    }
    else
    {
        info->framebuffer_count = min(MAX_FRAMEBUFFERS, fb_request.response->framebuffer_count);
        for (int i = 0; i < info->framebuffer_count; i++)
        {
            struct limine_framebuffer *fb = fb_request.response->framebuffers[i];
            info->framebuffer[i].address = (vaddr_t)fb->address;
            info->framebuffer[i].height = fb->height;
            info->framebuffer[i].width = fb->width;
            info->framebuffer[i].pitch = fb->pitch;
            info->framebuffer[i].bpp = fb->bpp;
        }
    }
}
