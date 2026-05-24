#pragma once

#include "types.h"

typedef struct {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) idt_entry;

typedef struct 
{
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) idtr;


void idt_init();
void sti();
void lidt(idtr *);