#include "types.h"
#include "debug.h"

typedef uint64_t gdt_entry_t;

typedef struct
{
    uint32_t base;
    uint32_t limit;
    uint8_t acess_byte;
    uint8_t flag;
} gdt;

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
    result |= (gdt_entry_t)(entry.acess_byte) << 40;

    // bit 39-16 is bottom 24 bits of base
    result |= (gdt_entry_t)(entry.base & 0xFFFFFF) << 16;

    // bit 15-0 is bottom 16 bits of limit
    result |= (gdt_entry_t)(entry.limit & 0xFFFF);

    return result;
}

void gdt_init()
{
    k_log("Initializing GDT");
}