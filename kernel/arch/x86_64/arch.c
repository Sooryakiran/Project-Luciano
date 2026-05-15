#include "arch.h"
#include "debug.h"
#include "arch/x86_64/gdt.h"

void arch_init() {
    k_log("Arch init x86\n");
    gdt_init();
}