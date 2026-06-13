#pragma once

#ifdef __X86_64__
#include "arch/x86_64/io.h"
#endif

#ifdef UNIT_TEST
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

static inline uint16_t inw(uint16_t port) 
{
    (void)port;
    return 0;
}
#endif