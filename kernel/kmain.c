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
#include "drivers/framebuffer/font8x16.h"
#include "scheduler.h"

static volatile LIMINE_BASE_REVISION(2);

__attribute__((noinline)) void process_a()
{
    k_log("A STARTING....");
    uint8_t i = 0;
    while (1)
    {
        i += 1;
        // k_log("A%d", i);
    }
}

__attribute__((noinline)) void process_b()
{
    k_log("B starting....");
    while (1)
    {
        // k_log("B");
    }
}

__attribute__((noinline)) void process_c()
{
    k_log("C starting....");
    while (1)
    {
        // k_log("C");
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
    scheduler_init();

    address_space_t curr_space = vmm_get_current_space();
    address_space_t c_space = vmm_create();
    process_t *proc_a = create_process((vaddr_t)&process_a, curr_space);
    process_t *proc_b = create_process((vaddr_t)&process_b, curr_space);
    process_t *proc_c = create_process((vaddr_t)&process_c, c_space);

    scheduler_add(proc_a);
    scheduler_add(proc_b);
    scheduler_add(proc_c);
    // scheduler_start();


    // process_switch(proc_a, proc_b);


    fb_draw_pixel(0, 100, 100, 0x00FF0000);
    // draw a pattern to confirm x/y orientation
    for (int i = 0; i < 100; i++)
    {
        fb_draw_pixel(0, i, 0, 0x00FF0000); // red horizontal line at top
        fb_draw_pixel(0, 0, i, 0x0000FF00); // green vertical line at left
    }

    fb_fill_rect(0, 30, 10, 100, 200, 0x00FFF0AB);
    fb_putchar(0, 0, 0, 'a', 0x00FFF0AB, 0x00FF0000, font_8x16_get());
    
    for (;;)
    {
    }
}
