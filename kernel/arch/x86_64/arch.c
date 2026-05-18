#include "arch.h"
#include "debug.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"

void arch_init() {
    k_log("[ARCH] Arch init x86");
    gdt_init();
    idt_init();
    pic_init();
    sti();
}