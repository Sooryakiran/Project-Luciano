#pragma once
#include "types.h"

#ifndef UNIT_TEST

#if defined(__x86_64__) || defined(__i386__)
#define software_interrupt_raise(n) asm volatile("int $" #n);
#endif
#else 

#define software_interrupt_raise(n) NULL;
#endif