#include "arch/x86_64/pic.h"
#include "io.h"
#include "debug.h"

// https://wiki.osdev.org/8259_PIC

#define MASTER_CMD_PORT 0x20
#define MASTER_DATA_PORT 0x21
#define SLAVE_CMD_PORT 0xA0
#define SLAVE_DATA_PORT 0xA1
#define CMD_INIT 0x11
#define EOI 0x20

void pic_init() {
    k_log("[PIC] Initializing PIC");

    // mask all first
    outb(MASTER_DATA_PORT, 0xFF);
    outb(SLAVE_DATA_PORT, 0xFF);

    // ICW1
    outb(MASTER_CMD_PORT, CMD_INIT);
    outb(SLAVE_CMD_PORT, CMD_INIT);

    // ICW2
    outb(MASTER_DATA_PORT, 32);
    outb(SLAVE_DATA_PORT, 40);

    // ICW3
    outb(MASTER_DATA_PORT, 0x04);
    outb(SLAVE_DATA_PORT, 0x02);

    // ICW4
    outb(MASTER_DATA_PORT, 0x01); 
    outb(SLAVE_DATA_PORT, 0x01); 

    // unmask
    outb(MASTER_DATA_PORT, 0);
    outb(SLAVE_DATA_PORT, 0);

    k_log("[PIC] PIC Initialized");
}

void pic_eoi(uint64_t vector) {
    if (vector >= 32 && vector <= 47) {
        if (vector >= 40) {
            // k_log("IRQ SLAVE EOI");
            outb(SLAVE_CMD_PORT, EOI);
        }
        // k_log("IRQ MASTER EOR");
        outb(MASTER_CMD_PORT, EOI);
    }
}