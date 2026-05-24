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
#include "string.h"

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

void process_d()
{
    fb_draw_pixel(0, 100, 100, 0x00FF0000);
    // draw a pattern to confirm x/y orientation
    for (int i = 0; i < 100; i++)
    {
        fb_draw_pixel(0, i, 0, 0x00FF0000); // red horizontal line at top
        fb_draw_pixel(0, 0, i, 0x0000FF00); // green vertical line at left
    }

    uint32_t x = 0;
    uint32_t dx = 1;
    uint32_t max_x = 1024;

    uint32_t color = 0x64;
    while (1)
    {
        x = (x + dx);
        if (x > max_x)
        {
            fb_fill_rect(0, 0, 10, max_x + 100, 200, 0);
            x = x % max_x;
            color = color + 64;
        }
        fb_fill_rect(0, x, 10, 100, 200, color);
    }

    // fb_putchar(0, 0, 0, 'a', 0x00FFF0AB, 0x00FF0000, font_8x16_get());
}

void process_e()
{
    fb_draw_pixel(0, 100, 100, 0x00FF0000);
    // draw a pattern to confirm x/y orientation
    for (int i = 0; i < 100; i++)
    {
        fb_draw_pixel(0, i, 0, 0x00FF0000); // red horizontal line at top
        fb_draw_pixel(0, 0, i, 0x0000FF00); // green vertical line at left
    }

    uint32_t x = 0;
    uint32_t dx = 1;
    uint32_t max_x = 1024;

    uint32_t color = 0x64;
    while (1)
    {
        x = (x + dx);
        if (x > max_x)
        {
            fb_fill_rect(0, 0, 400, max_x + 100, 200, 0);
            x = x % max_x;
            color = color - 64;
        }
        fb_fill_rect(0, x, 400, 100, 200, color);
        if (x % 8 == 0)
        {
            fb_putchar(0, x, 600, 'a', 0x00FFF0AB, 0x00FF0000, font_8x16_get());
        }
    }
}

void user_task_a()
{
    while (1)
    {
        // just spin, no kernel calls
    }
}

extern uint8_t user_loop_start[];
extern uint8_t user_loop_end[];

process_t *load_user_loop(boot_info *info)
{
    
    address_space_t space = vmm_create();
    size_t size = user_loop_end - user_loop_start;
    vaddr_t user_code_va = 0x00400000;

    // map pages at user VA
    size_t num_frames = (size + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;

    k_log("[USER] size=%d frames=%d", size, num_frames);
    for (size_t i = 0; i < num_frames; i++)
    {
        paddr_t phys = pmm_alloc();
        vmm_map(space, user_code_va + i * PMM_FRAME_SIZE, phys, 0x7);
        // copy binary bytes to physical page via hhdm
        size_t offset = i * PMM_FRAME_SIZE;
        size_t copy_size = (offset + PMM_FRAME_SIZE > size) ? (size - offset) : PMM_FRAME_SIZE;
        memcpy((void *)(phys + info->hhdm_offset), user_loop_start + offset, copy_size);
    }
    process_t *proc = create_process(user_code_va, space, PRIVILEGE_USER);
    task_t *task_2 = task_create(proc, user_code_va);
    process_add_task(proc, task_2);
    return proc;
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
    process_t *proc_a = create_process((vaddr_t)&process_a, curr_space, PRIVILEGE_KERNEL);
    task_t *task_b = task_create(proc_a, (vaddr_t)&process_b);
    process_add_task(proc_a, task_b);
    // process_t *proc_b = create_process((vaddr_t)&process_b, curr_space);
    process_t *proc_c = create_process((vaddr_t)&process_c, c_space, PRIVILEGE_KERNEL);
    process_t *proc_d = create_process((vaddr_t)&process_d, curr_space, PRIVILEGE_KERNEL);
    process_t *proc_e = create_process((vaddr_t)&process_e, curr_space, PRIVILEGE_KERNEL);

    process_t *user_proc_a = load_user_loop(&info);
    process_t *user_proc_b = load_user_loop(&info);


    scheduler_add(proc_a->tasks[0]);
    scheduler_add(proc_a->tasks[1]);
    // scheduler_add(proc_b->tasks[0]);
    scheduler_add(proc_c->tasks[0]);
    scheduler_add(proc_d->tasks[0]);
    scheduler_add(proc_e->tasks[0]);
    
    scheduler_add(user_proc_a->tasks[0]);
    scheduler_add(user_proc_a->tasks[1]);

    // scheduler_add(user_proc_b->tasks[0]);
    // scheduler_add(user_proc_b->tasks[1]);

    // scheduler_add(user_proc_a->tasks[1]);


    // process_switch(proc_a, proc_b);
    scheduler_enable();
    for (;;)
    {
    }
}
