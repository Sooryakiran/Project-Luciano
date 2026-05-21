#include "pmm.h"
#include "debug.h"

#define ALIGN_FRAME(pa) (((pa) + (PMM_FRAME_SIZE - 1)) & ~((uint64_t)PMM_FRAME_SIZE - 1))
#define ALIGN_FRAME_DOWN(pa) ((pa) & ~((uint64_t)PMM_FRAME_SIZE - 1))

uint64_t get_total_memory_size(mem_region *regions, size_t count)
{
    uint64_t top = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (!(regions[i].type == MEM_USABLE || regions[i].type == MEM_KERNEL)) continue;
        uint64_t end_region = regions[i].offset + regions[i].size;
        if (end_region > top)
            top = end_region;
    }
    return top;
}

static uint64_t pmm_total_frames;
static uint64_t pmm_bitmap_size;
static uint8_t *pmm_bitmap;
static uint64_t pmm_physical;

paddr_t pmm_alloc()
{
    for (uint64_t i = 0; i < pmm_bitmap_size; i++)
    {
        if (pmm_bitmap[i] == 0xFF)
            continue;

        for (int j = 0; j < 8; j++)
        {
            if (((1 << j) & pmm_bitmap[i]) != 0)
                continue;
            pmm_bitmap[i] |= (1 << j);
            return (i * 8 + j) * PMM_FRAME_SIZE;
        }
    }

    // TODO: return null and let paging handle LRU eviction to swap etc. 
    // TODO: don't panic that time
    k_panic("[PMM] no place to allocate");
    return 0;
}

void pmm_free(paddr_t addr) {
    k_log("[PMM] Freeing memory.");
    uint64_t frame = addr / PMM_FRAME_SIZE;
    uint8_t j = frame % 8;
    uint64_t i = frame / 8;
    pmm_bitmap[i] &= ~(1 << j);
    k_log("[PMM] Memory freed.");
}

void pmm_init(mem_region *regions, size_t count, uint64_t hhdm_offset)
{
    k_log("[PMM] Initializing physical memory maps");

    uint64_t total_memory = get_total_memory_size(regions, count);
    pmm_total_frames = (total_memory + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;
    pmm_bitmap_size = (pmm_total_frames + 7) / 8;
    k_log("[PMM] bitmap_size is %lu", pmm_bitmap_size);

    // find usable region
    for (size_t i = 0; i < count; i++)
    {
        if (regions[i].type == MEM_USABLE && regions[i].size > pmm_bitmap_size) // we are comparing in bytes
        {
            k_log("[PMM] Found free region to allocate pmm_bitmap");
            pmm_physical = ALIGN_FRAME(regions[i].offset);
            pmm_bitmap = (uint8_t *)(pmm_physical + hhdm_offset);
            break;
        }
    }

    // mark all frames as used
    k_log("[PMM] Marking all regions as used.");
    for (uint64_t i = 0; i < pmm_bitmap_size; i++)
    {
        pmm_bitmap[i] = 0xFF;
    }
    k_log("[PMM] Done marking all regions as used.");

    // clear free regions
    k_log("[PMM] Clearning regions that are free");
    for (size_t i = 0; i < count; i++)
    {
        if (regions[i].type != MEM_USABLE)
            continue;
        uint64_t base = ALIGN_FRAME(regions[i].offset);
        uint64_t end = ALIGN_FRAME_DOWN(regions[i].offset + regions[i].size);
        uint64_t frame_start = base / PMM_FRAME_SIZE;
        uint64_t frame_count = (end - base) / PMM_FRAME_SIZE;
        for (uint64_t f = frame_start; f < frame_start + frame_count; f++)
        {
            // we store 8 bits in a uint8
            // we take the frame remainder, eg if 7th: we get 01000000,
            // and we invert it 1011111, and & with the existing value,
            // to clear the particular bit
            pmm_bitmap[f / 8] &= ~(1U << (f % 8));
        }
    }
    k_log("[PMM] Done clearing free regions.");

    // unclear bitmap allocated region
    k_log("[PMM] Marking bitmap used region as used");
    uint64_t bitmap_frame_start = (uint64_t)pmm_physical / PMM_FRAME_SIZE;
    uint64_t bitmap_frame_count = (pmm_bitmap_size + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;
    for (uint64_t f = bitmap_frame_start; f < bitmap_frame_start + bitmap_frame_count; f++)
    {
        pmm_bitmap[f / 8] |= (1U << (f % 8));
    }
    k_log("[PMM] Done marking bitmap used region as used");
    k_log("[PMM] total frames %lu", pmm_total_frames);
}

uint64_t pmm_get_total_memory() {
    return pmm_total_frames * PMM_FRAME_SIZE;
}