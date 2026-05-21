#include <limine.h>
#include "debug.h"
#include "arch.h"
#include "boot.h"
#include "pmm.h"
#include "vmm.h"
#include "kmalloc.h"
#include "process.h"
#include "drivers.h"
#include "drivers/framebuffer/fb.h"

static volatile LIMINE_BASE_REVISION(2);

__attribute__((noinline)) void process_a()
{
    while (1)
    {
        k_log("A\n");
    }
}

__attribute__((noinline)) void process_b()
{
    while (1)
    {
        k_log("B\n");
    }
}

void kmain(void)
{

    k_init();
    k_log("Hello, World!");

    boot_info info;
    get_boot_entry(&info);

    arch_init();
    pmm_init(info.regions, info.region_count, info.hhdm_offset);
    vmm_init(info.kernel_physical_addr, info.kernel_virtual_addr, info.hhdm_offset, info.regions, info.region_count);
    kmalloc_init();
    drivers_init(&info);

    fb_draw_pixel(0, 100, 100, 0x00FF0000);
    // draw a pattern to confirm x/y orientation
    for (int i = 0; i < 100; i++)
    {
        fb_draw_pixel(0, i, 0, 0x00FF0000); // red horizontal line at top
        fb_draw_pixel(0, 0, i, 0x0000FF00); // green vertical line at left
    }

    fb_fill_rect(0, 30, 10, 100, 200, 0x00FFF0AB);

    void *ptr = kmalloc(8192);
    k_log("[TEST] kmalloc returned %x", (uint64_t)ptr);
    void *ptr2 = kmalloc(100);
    k_log("[TEST] kmalloc returned %x", (uint64_t)ptr2);

    // in kmain or wherever:
    // address_space_t current_address_space = vmm_get_current_space();
    // process_t *a = create_process((vaddr_t)process_a, current_address_space);
    // process_t *b = create_process((vaddr_t)process_b, current_address_space);

    // process_switch(a, b); // should start printing B

    for (;;)
    {
    }
}
