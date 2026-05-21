#include "arch/x86_64/pit.h"
#include "debug.h"
#include "types.h"

#define FREQ_4_MS 4772

void pit_init()
{
    k_log("[PIT] Initializing PIT");
    uint16_t divisor = FREQ_4_MS;
    __asm__ volatile(
        "mov $0x36, %%al\n\t"
        "out %%al, $0x43\n\t"
        "out %%al, $0x40\n\t"
        "mov %%ah, %%al\n\t"
        "out %%al, $0x40\n\t"
        :
        : "a"(divisor)
        :);
    
}