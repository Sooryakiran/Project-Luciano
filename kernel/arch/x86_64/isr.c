#include "debug.h"
#include "types.h"
#include "arch/x86_64/pic.h"
#include "arch/x86_64/io.h"

void exception_handler(uint64_t, uint64_t);
void irq_handler(uint64_t);
void timer_handler();
void keyboard_handler();

void exception_handler(uint64_t vector, uint64_t error_code) {
    if (vector >= 32 && vector <= 47) {
        irq_handler(vector);
        return;
    }
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    
    k_log("[ISR] vector %lu (0x%x)", vector, vector);
    k_log("[ISR] error code %lu (0x%x)", error_code, error_code);
    k_log("[ISR] cr2 = %x", cr2);
    k_panic("[ISR] exception occured");
    while (1) {}
}

void irq_handler(uint64_t vector) {
    switch (vector)
    {
    case 32:
        timer_handler();
        break;
    case 33:
        keyboard_handler();
        break;
    default:
        break;
    }
    pic_eoi(vector);

}

void timer_handler() {
    // k_log("Timer Interrupt");
}

void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    k_log("[ISR] Keyboard handler");
}