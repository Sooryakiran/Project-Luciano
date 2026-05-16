#include "arch/x86_64/idt.h"
#include "debug.h"

uint8_t IDT_MAX_DESCRIPTORS = 32;

uint16_t GDT_OFFSET_KERNEL_CODE = 0x08;

extern void* isr_stub_table[];

idt_entry create_idt_entry(void* offset, uint8_t flags) {
    idt_entry descriptor;
    descriptor.offset_low = (uint64_t)offset & 0xffff;
    descriptor.segment_selector = GDT_OFFSET_KERNEL_CODE;
    descriptor.ist = 0;
    descriptor.attributes = flags;
    descriptor.offset_mid = ((uint64_t)offset >> 16) & 0xffff;
    descriptor.offset_high = ((uint64_t)offset >> 32) & 0xffffffff;
    descriptor.zero = 0; 
    return descriptor;
}

void idt_init() {
    k_log("Initializing IDT");

    __attribute__((aligned(0x10))) 
    static idt_entry idt[256];

    for(uint8_t i = 0; i < IDT_MAX_DESCRIPTORS; i++) {
        idt[i] = create_idt_entry(isr_stub_table[i], 0x8E);
    }

    static idtr registry;
    registry.offset = (uint64_t)idt;
    registry.size = (uint16_t)(sizeof(idt)) - 1;

    __asm__ volatile ("lidt %0" : : "m"(registry));
    __asm__ volatile ("sti");

    k_log("IDT set successfully");
}