#include "types.h"
#include "debug.h"
#include "arch/x86_64/gdt.h"

// reference for myself: https://wiki.osdev.org/Global_Descriptor_Table

extern void flush_gdt();

gdt_entry_t create_gdt_entry(gdt entry)
{
    if (entry.limit > 0xFFFFF)
        k_panic("GDT limit cannot be more than 20 bits");
    if (entry.flag > 0xF)
        k_panic("GDT flag cannot be more than 4 bits");

    gdt_entry_t result = 0;

    // bit 63-56 is base 31 to 24
    result |= ((gdt_entry_t)(entry.base & 0xFF000000) << 32);

    // bit 55-52 is flag
    result |= ((gdt_entry_t)(entry.flag & 0xF) << 52);

    // bit 48-51 is top 4 bits of limit
    result |= (gdt_entry_t)((entry.limit >> 16) & 0xF) << 48;

    // bit 40-47 is access byte
    result |= (gdt_entry_t)(entry.access_byte) << 40;

    // bit 39-16 is bottom 24 bits of base
    result |= (gdt_entry_t)(entry.base & 0xFFFFFF) << 16;

    // bit 15-0 is bottom 16 bits of limit
    result |= (gdt_entry_t)(entry.limit & 0xFFFF);

    return result;
}

#ifndef UNIT_TEST

void load_gdt(gdtr* registry) {
    asm volatile ("lgdt %0" : : "m"(*registry));
}

void gdt_init()
{
    k_log("[GDT] Initializing GDT");
    static gdt_entry_t gdt_table[3];
    static gdtr registry;
    gdt_table[0] = create_gdt_entry((gdt){0, 0, 0, 0});            // NULL Descriptor
    gdt_table[1] = create_gdt_entry((gdt){0, 0xFFFFF, 0x9a, 0xa}); // Kernel code, all
    gdt_table[2] = create_gdt_entry((gdt){0, 0xFFFFF, 0x92, 0xc}); // Kernel data, all
    // todo: after user mode, and after interrupts

    registry.offset = (uint64_t)gdt_table;
    registry.size = sizeof(gdt_table) - 1;
    
    load_gdt(&registry);
    flush_gdt();
    k_log("[GDT] GDT initialized!");

}

#else
void gdt_init() {}
#endif