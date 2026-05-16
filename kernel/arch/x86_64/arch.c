#include "arch.h"
#include "debug.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"

void arch_init() {
    k_log("Arch init x86");
    gdt_init();
    idt_init();
}