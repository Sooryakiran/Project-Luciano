#pragma once
#include "types.h"

#ifndef UNIT_TEST

// returns faulting address
static inline uint64_t crx_read_cr2()
{
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

// returns physical address of page table root
static inline uint64_t crx_read_cr3() 
{
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

// writes a physical address to page root table
static inline void crx_write_cr3(paddr_t physical_addr) {
    asm volatile(
        "mov %0, %%cr3"      // %0 = first input, %% escapes to literal % for registers
        :                    // output operands (none)
        : "r"(physical_addr) // input: "r" = put in any general purpose register, (phys) = the value
        : "memory"           // clobber: tell compiler memory may have changed
    );
}

#else
static inline uint64_t crx_read_cr2() { return 0; }
static inline uint64_t crx_read_cr3() { return 0; }
static inline void crx_write_cr3(paddr_t physical_addr) {}
#endif