#pragma once

#include "types.h"

#ifndef UNIT_TEST

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

#else

// for unit tests, i don't have an x86 machine, so skipping on my arm mac
static inline void outb(uint16_t port, uint8_t val)
{
    (void)port;
    (void)val;
}

static inline uint8_t inb(uint16_t port)
{
    (void)port;
    return 0;
}
#endif
