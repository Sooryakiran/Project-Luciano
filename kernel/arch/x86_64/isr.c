#include "debug.h"
#include "types.h"
#include "arch/x86_64/pic.h"
#include "arch/x86_64/io.h"

void exception_handler(uint64_t, uint64_t);
void irq_handler(uint64_t);
void timer_handler();
void keyboard_handler();

void exception_handler(uint64_t vector, uint64_t error_code) {
    char msg[3];
    msg[0] = "0123456789ABCDEF"[(vector >> 4) & 0xF];
    msg[1] = "0123456789ABCDEF"[vector & 0xF];
    msg[2] = '\0';

    char err_msg[3];
    err_msg[0] = "0123456789ABCDEF"[(error_code >> 4) & 0xF];
    err_msg[1] = "0123456789ABCDEF"[error_code & 0xF];
    err_msg[2] = '\0';

    if (vector >= 32 && vector <= 47) {
        irq_handler(vector);
        return;
    }
    
    k_log("[ISR] vector");
    k_log(msg);
    k_log("[ISR] error code");
    k_log(err_msg);
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